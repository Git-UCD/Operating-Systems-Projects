#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <cstdlib>

int main(void){
  int     fd[2], nbytes;
  pid_t   childpid;
  char    string[] = "ls";
  char    readbuffer[80];
  pipe(fd);
  
  if((childpid = fork()) == -1){
    perror("fork");
    exit(1);
  }

  if(childpid == 0){
    dup2(STDOUT_FILENO,fd[0]);
    write(fd[1], string, (strlen(string)+1));
    exit(0);
  }
  else{
    close(fd[1]); // parent closes write
//  #bytes read   input  store here  read in this much
    nbytes = read(fd[0], readbuffer, sizeof(readbuffer));
    printf("Received string: %s of size %d", readbuffer, nbytes);
  }
  return 0;
}
