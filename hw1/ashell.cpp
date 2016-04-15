#include <sys/types.h> // includes headers
#include <sys/stat.h> // obtain information about file using stat()
#include <iostream>
#include <stdio.h>
#include <errno.h>
#include <dirent.h> //includes typedef DIR (type rep directory stream)
#include <unistd.h>
#define GetCurrentDir getcwd
using namespace std;

int main(){
  /* Immediately get the current working directory, this code is pulled from http://stackoverflow.com/questions/143174/how-do-i-get-the-directory-that-a-program-is-running-from on 3/29/2016 by original user computinglife and edited by Polar*/
  char cCurrentPath[FILENAME_MAX];
  if(!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath))){
    return errno;}
  cout << "Current working directory: " << cCurrentPath << endl;
  cout << "Changing directory as: cd .." << endl;
  chdir("..");
  if(!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath))){
    return errno;}
  cout << "Current working directory: " << cCurrentPath << endl;
  //end print current directory testing

  //begin "ls" testing
  /*
   * Heavily influenced by http://stackoverflow.com/questions/13554150/implementing-the-ls-al-command-in-c, accessed 3/29/16, written by user hbteibet
   * File permission checking "if blocks" direct copy of:
   * http://stackoverflow.com/questions/10323060/printing-file-permissions-like-ls-l-using-stat2-in-c
   * linked by user askovpen on 3/29/2016
   */
  DIR *mydir;
  struct dirent *myfile;
  struct stat mystat;
  char buf[512];
  mydir = opendir("./ecs150/");
  // BUG: for ".", it only prints -------... when it should print drwx--...
  while((myfile = readdir(mydir)) != NULL){
    printf((S_ISDIR(mystat.st_mode)) ? "d" : "-");
    printf((mystat.st_mode & S_IRUSR) ? "r" : "-");
    printf((mystat.st_mode & S_IWUSR) ? "w" : "-");
    printf((mystat.st_mode & S_IXUSR) ? "x" : "-");
    printf((mystat.st_mode & S_IRGRP) ? "r" : "-");
    printf((mystat.st_mode & S_IWGRP) ? "w" : "-");
    printf((mystat.st_mode & S_IXGRP) ? "x" : "-");
    printf((mystat.st_mode & S_IROTH) ? "r" : "-");
    printf((mystat.st_mode & S_IWOTH) ? "w" : "-");
    printf((mystat.st_mode & S_IXOTH) ? "x" : "-");



    sprintf(buf, "%s" ,myfile->d_name);
    stat(buf, &mystat);
    printf(" %s\n", myfile->d_name);
  }





  return 0;
}
