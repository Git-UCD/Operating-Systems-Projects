#include <unistd.h>
#include <sys/wait.h>

int main(){
  pid_t pid;
  char *argvv[2][3] = {{"ls",0,0},{"grep","a",0}};
  int status;

  int fd[2];
  pipe(fd);

  if ((pid = fork()) != -1){
    if(pid == 0){
      //close(fd[0]);
      close(1);
      dup(fd[1]);
      //dup2(STDOUT_FILENO, fd[0]);
      execvp(argvv[0][0], argvv[0]);
      close(fd[1]);
    }

    else{
      wait(&status);
      close(fd[1]);
      close(0);
      dup(fd[0]);
      close(fd[0]);
      execvp(argvv[1][0], argvv[1]);
    }
  }
  return 0;
}
