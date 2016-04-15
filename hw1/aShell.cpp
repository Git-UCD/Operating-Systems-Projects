#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <ctype.h>

#include <errno.h>
#include <dirent.h> //includes typedef DIR (type rep directory stream)
#include <sys/types.h> // includes headers
#include <sys/stat.h> // obtain information about file using stat()

#include <iostream>
#include <string>
#include <string.h>
#define GetCurrentDir getcwd //changes function name to something readable, read below for citation, picked this up from SO

using namespace std;

int pwd();

void reset_shell( int fd, struct termios *savedattributes){
	tcsetattr( fd, TCSANOW,savedattributes);
}

void start_shell(int fd, struct termios *savedattributes){
	struct termios TermAttributes;
	char *name;
	// Make sure stdin is a terminal
	if(!isatty(fd)){
		fprintf(stderr,"Not a terminal.\n");
		exit(0);
	}
	// Save the terminal attributes
	tcgetattr(fd,savedattributes);

	// save terminal modes
	tcgetattr(fd,&TermAttributes);
	
  TermAttributes.c_lflag = (ICANON | ECHO); /*local modes*/ 
	tcsetattr(fd,TCSAFLUSH,&TermAttributes);
}
void ls(){
	DIR *mydir;
	struct dirent *myfile;
	struct stat mystat;
	char buf[512];
  pwd();
	mydir = opendir("/home/w1155959/ecs150/");

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

	    sprintf(buf,"%s", myfile->d_name);
	    stat(buf,&mystat);
	    printf("%s\n", myfile->d_name);

	}
}
int pwd(){
	 /* Immediately get the current working directory, this code is pulled from http://stackoverflow.com/questions/143174/how-do-i-get-the-directory-that-a-program-is-running-from on 3/29/2016 by original user computinglife and edited by Polar*/
  char cCurrentPath[FILENAME_MAX];
  if(!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath))){
    return errno;}
  cout << "Current working directory: " << cCurrentPath << endl;
  return 0;

}

int main(){
	struct termios termModeAttributes;
	//opengroup.org 
	char buf [255];
	size_t nbytes = sizeof(buf);
	ssize_t rbytes;




  //STDIN_FILENO is defined as 0 in unistd.h
	start_shell(STDIN_FILENO,&termModeAttributes);

	while(1){
		rbytes = read(STDIN_FILENO,buf,nbytes); // number of bytes read
		buf[rbytes] = 0;
		std::string inputStr(buf,strlen(buf)-1);
		
		if ( inputStr == "exit" ){
			break;
		}
		else if (inputStr == "pwd"){
			pwd();
		}
		else if (inputStr == "ls"){
			ls();
		}

	}
	reset_shell(STDIN_FILENO,&termModeAttributes);
	return 0;
}
