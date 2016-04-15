#include <unistd.h>
#include <cstdlib>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
using namespace std;


void    loop_pipe(char ***cmd) 
{
  int   p[2];
  int child = 0;
  pid_t pid;
  int   fd_in = 0;

  while (*cmd != NULL)
  {
    pipe(p);
    //cout << "Child: " << ++child << endl;
    //cout << "p[0]: " << p[0] << " p[1]: " << p[1] << endl;
    if ((pid = fork()) == -1)
    {
      exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
      //cout << "dup2("<<fd_in<<", 0)" << endl;
      dup2(fd_in, 0); //change the input according to the old one 
      //cout << (*cmd)[0] << endl;
      if (*(cmd + 1) != NULL){
        //cout << "dup2(" << p[1] <<", 1)" << endl;
        dup2(p[1], 1);
      }
      //cout << "close("<<p[0]<<endl;
      close(p[0]);
      cout << "RUN: " << (*cmd)[0] << " " << *cmd << endl;
      execvp((*cmd)[0], *cmd);
      exit(EXIT_FAILURE);
    }
    else
    {
      wait(NULL);
      cout << endl << "In Parent" << endl;
      //cout << "close(" << p[1] << ")" << endl;
      close(p[1]);
      fd_in = p[0]; //save the input for the next command
      //cout << "fd_in = " <<  p[0] << endl;
      cmd++;
    }
  }
}

int main()
{
  char *ls[] = {"ls", NULL};
  char *grep[] = {"grep", "pipe", NULL};
  char *wc[] = {"wc", NULL};
  char **cmd[] = {ls, grep, wc, NULL};

  loop_pipe(cmd);
  return (0);
}


