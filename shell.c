#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <fcntl.h>

#define INPUT_STRING_SIZE 80
//#define LOCKFILE "/etc/ptmp"

#include "io.h"
#include "parse.h"
#include "process.h"
#include "shell.h"

int cmd_quit(tok_t arg[]) {
  printf("Bye\n");
  exit(0);
  return 1;
}

int cmd_help(tok_t arg[]);
int cmd_cd(tok_t arg[]);
char *kuda(char *a,char *b){
  char *cat=malloc(strlen(a)+strlen(b)+1);
  strcpy(cat,a);
  strcat(cat,b);
  return cat;
}
/* Command Lookup table */
typedef int cmd_fun_t (tok_t args[]); /* cmd functions take token array and return int */
typedef struct fun_desc {
  cmd_fun_t *fun;
  char *cmd;
  char *doc;
} fun_desc_t;

fun_desc_t cmd_table[] = {
  {cmd_help, "?", "show this help menu"},
  {cmd_quit, "quit", "quit the command shell"},
  {cmd_cd,"cd","go to a directory"},
};

int cmd_help(tok_t arg[]) {
  int i;
  for (i=0; i < (sizeof(cmd_table)/sizeof(fun_desc_t)); i++) {
    printf("%s - %s\n",cmd_table[i].cmd, cmd_table[i].doc);
  }
  return 1;
}
tok_t *Paths(){
  char * h = getenv("PATH");
  tok_t *r = getToks(h);
  return r;
}
void findPath(tok_t *ka){

  tok_t *f= Paths();
  char * de;
  int j;
  for(j=0;j<MAXTOKS&&f[j];j++){
    de=kuda(f[j],"/");
    if(access(kuda(de,ka[0]),F_OK)!=-1){
      execve(kuda(de,ka[0]),ka,NULL);
    }
  }
}
void Command(tok_t *t,int ope,int i){
  printf("%s","shmuck");
	    if(strcmp(t[i],">")==0){
	      t[i]=NULL;
	      if(dup2(ope,1)!=-1){
		findPath(t);
		close(ope);
	      }
	    }else if(strcmp(t[i],"<")==0){
	      t[i]=NULL;
	      if(dup2(ope,0)!=-1){
		findPath(t);  
		close(ope);
	      }
	    }
}
int cmd_cd(tok_t arg[]) {
  chdir(arg[0]);
  /*
    char *k;
    k+=arg[0];
    printf("%s ",k);
    return 1;*/
  return 0;
}
int lookup(char cmd[]) {
  int i;
  for (i=0; i < (sizeof(cmd_table)/sizeof(fun_desc_t)); i++) {
    if (cmd && (strcmp(cmd_table[i].cmd, cmd) == 0)) return i;
  }
  return -1;
}

void init_shell()
{
  /* Check if we are running interactively */
  shell_terminal = STDIN_FILENO;
  
  /** Note that we cannot take control of the terminal if the shell
      is not interactive */
  shell_is_interactive = isatty(shell_terminal);
  
  if(shell_is_interactive){
    
    /* force into foreground */
    while(tcgetpgrp (shell_terminal) != (shell_pgid = getpgrp()))
      kill( - shell_pgid, SIGTTIN);
    
    shell_pgid = getpid();
    /* Put shell in its own process group */
    if(setpgid(shell_pgid, shell_pgid) < 0){
      perror("Couldn't put the shell in its own process group");
      exit(1);
    }
    
    /* Take control of the terminal */
    tcsetpgrp(shell_terminal, shell_pgid);
    tcgetattr(shell_terminal, &shell_tmodes);
  }
  /** YOUR CODE HERE */
}

/**
 * Add a process to our process list
 */
void add_process(process* p)
{
  /** YOUR CODE HERE */
}

/**
 * Creates a process given the inputString from stdin
 */
process* create_process(char* inputString)
{
  /** YOUR CODE HERE */
  return NULL;
}



int shell (int argc, char *argv[]) {
  char *s = malloc(INPUT_STRING_SIZE+1);			/* user input string */
  tok_t *t;			/* tokens parsed from input */
  int lineNum = 0;
  int fundex = -1;
  pid_t pid = getpid();		/* get current processes PID */
  pid_t ppid = getppid();	/* get parents PID */
  pid_t cpid, tcpid, cpgid;
  char a[200]="";
  
  init_shell();
  printf("%s running as PID %d under %d\n",argv[0],pid,ppid);
  
  lineNum=0;
  fprintf(stdout, "%d %s: ", lineNum, getcwd(*a, 200));
  while ((s = freadln(stdin))){
    t = getToks(s); /* break the line into tokens */
    //	fprintf(stdout,"%s\n",t[0])
    fundex = lookup(t[0]); /* Is first token a shell literal */
    // int st=-1;
    /* if(strncmp(gt,">",1)==0/*||strcmp(strstr(s,"<"),"<")==0*){
       st=0;
       /*  fprintf(stdout, "%d %s: ", lineNum,"GOT IT");
       }
       if(st!=-1){*/
    if(fundex >= 0) cmd_table[fundex].fun(&t[1]);
    else {
      printf("Shmuck");
      pid=fork();
      if(pid == 0){
	char * d=kuda("/",t[0]);
	int j;
	int ope;
	int i;
	for(i=0;i<MAXTOKS&&t[i];i++){
	  if(strcmp(t[i],"<")==0){
	    ope=open(t[i+1], O_RDONLY/*,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH*/);
	    Command(t,ope,i);
	    i=MAXTOKS&&t[i];
	  }else if(strcmp(t[i],">")==0){
	    ope=open(t[i+1], O_CREAT|O_WRONLY| O_APPEND/*,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH*/);
	    Command(t,ope,i);
	    i=MAXTOKS&&t[i];
	  }
	} 
	findPath(t);
	printf(d);
	execve(t[0],t,NULL);
	perror(" error");
	exit(0);
      }
      wait(NULL);
      /*      fprintf(stdout, "This shell only supports built-ins. Replace this to run programs as commands.\n");*/
    }
    lineNum++;
    fprintf(stdout, "%d %s: ", lineNum, getcwd(*a,200));
  }
  return 0;
}

