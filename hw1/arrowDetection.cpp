#include <unistd.h>
#include <stdio.h>
#include <sstream>
#include <stdlib.h>
#include <termios.h>
#include <ctype.h>
#include <vector>
#include <cstdlib>
#include <errno.h>
#include <dirent.h> //includes typedef DIR (type rep directory stream)
#include <sys/types.h> // includes headers
#include <sys/stat.h> // obtain information about file using stat()
#include <sys/wait.h>
#include <iostream>
#include <string>
#include <string.h>
#define GetCurrentDir getcwd
#define READ_END 0
#define WRITE_END 1
using namespace std;

/////////////// START PIPE.CPP

class command{
  public:
    string cmd;
    string opt;
};

void ls(string directory);

void tokenize(char *str, vector<string>& tkns){
  char *s;
  string ss;
  s = strtok(str, " ");
  while(s != NULL){
    ss = s;
    tkns.push_back(ss);
    s = strtok(NULL, " ");
  }
}

void getCmds(const vector<string> tkns, vector<command>& cmds){
  for(int i = 0; i < tkns.size(); ){
    //get primary command
    command *cmd = new command();
    cmd->cmd = tkns[i];
    //cout << "Cmd: " << tkns[i] << endl;
    //get options
    int addingOptions = 0;
    if(cmd->cmd != "|" && cmd->cmd != ">" && cmd->cmd != ">>" && cmd->cmd != "<"){
      while(i+1 < tkns.size() && tkns[i+1] != "|" && tkns[i+1] != "<" && tkns[i+1] != ">" && tkns[i+1] != ">>"){
        if(addingOptions){
          cmd->opt.append(" ");
        }
        cmd->opt.append(tkns[++i]);
        //cout << "Opt: " << tkns[i] << endl;
        addingOptions = 1;
      }
    }
    //if(cmd->cmd == "|" || cmd->cmd == ">" || cmd->cmd == ">>" || cmd->cmd == "<"){
    i++;
    //}
    //if(addingOptions == 1){
    //i++;
    //}
    cmds.push_back(*cmd);
  }
}

//returns number of commands, does no count |, < , <<, >
int numOfCommands(vector<command> cmds){
  int commands = 0;
  for(int i = 0; i < cmds.size(); i++){
    if(cmds[i].cmd != "|" && cmds[i].cmd != "<" && cmds[i].cmd != ">" && cmds[i].cmd != ">>") commands++;
  }
  return commands;
}

// use on a cmd to see how many options it has
int numOptions(vector<command> cmds, int i){
  if(!(cmds[i].opt.empty())){
    int opts = 0;
    char *dup = strdup(((cmds[i].opt).c_str()));
    char *s;
    s = strtok(dup, " ");
    while(s != NULL){
      opts++;
      s = strtok(NULL, " ");
    }
    free(dup);
    return opts;
  }
  else
    return 0;

}

void cmdToExecString(vector<command> cmds, int i){
  int lengthOfArg = numOptions(cmds,i);
  cout << "NUM OPT: " << lengthOfArg << endl;
  char *args = (char *) malloc(sizeof(char)*(2+lengthOfArg));
  //args[0] = (char *)cmds[i].cmd.c_str();
  //*ar
}

void loop_pipe(vector<command> cmds){
  int p[2];
  pid_t pid;
  int fd_in = 0;
  int commands = numOfCommands(cmds); // number of commands to run
  int child = 0;
  for(int i = 0; i < commands;){
    //cout << "fd_in: " << fd_in << endl;
    pipe(p);
    pid = fork();
    child++;
    if(pid == 0){
      //cout << "In new child" << endl;
      //cout << "old fd: " << fd_in << " new fd: 0 (closing old fd and copying it to 0: stdin)" << endl;
      dup2(fd_in, 0);
      //if we're not at the end of commands, pipe this child's ouput out to 1
      if(i+1 != commands){
        //cout << "setting fd: " << p[1] << " to 1" << endl;
        dup2(p[WRITE_END],STDOUT_FILENO);
      }
      //if(cmds[i+1].cmd == ">"){
      //  cout << "NEXT COMMAND IS > " << endl;
      //  open(const_cast<char *>((cmds[i+2].cmd).c_str())
      //  dup2(STDOUT_
      //}
      close(p[READ_END]);
      //if(child == 1){
        //using http://stackoverflow.com/questions/28712552/c-using-execvp-with-string-array to convert string to stupid execvp format
        while(cmds[i].cmd == "|"){
          i++;
        }
        int argc = numOptions(cmds,i)+1;
        char* args[argc];
        args[0] = const_cast<char *>((cmds[i].cmd).c_str());
        for(int x = 1; x < argc; x++){
          args[x] = const_cast<char *>((cmds[i].opt).c_str());
        } 
        args[argc] = NULL;
        if(cmds[i].cmd == "ls")
        {
          ls(cmds[i].opt);
        }
        else{
          execvp(args[0],args);
        }
      //}
      /*else{
        char* args[2];
        args[0] = "wc";
        args[1] = NULL;
        execvp(args[0], args);
      }*/
      exit(EXIT_FAILURE);
    }
    else{
      wait(NULL);
      //cout << "In parent" << endl;
      //cout << "closing " << p[1] << endl;
      close(p[WRITE_END]);
      //cout << "fd_in = " << p[0] << endl;
      fd_in = p[READ_END];
      i++;
    }
  }
}

//////////////// END PIPE.CPP

int pwd();
string getPwd();

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

  TermAttributes.c_lflag &= (ECHO); /*local modes*/ 
  TermAttributes.c_lflag &= ~(ICANON);
  TermAttributes.c_cc[VMIN] = 1;
  TermAttributes.c_cc[VTIME] = 0;
  tcsetattr(fd,TCSAFLUSH,&TermAttributes);
}
//ls(): http://stackoverflow.com/questions/20775762/how-to-walk-through-a-directory-in-c-and-print-all-files-name-and-permissions?lq=1
void ls(string directory){
  char *currentDir;
  if(directory.empty()){
    string currentDirectory = getPwd();
    currentDir = const_cast<char *>(currentDirectory.c_str());
  }
  else{
    currentDir = const_cast<char *>(directory.c_str());
  }
  //mydir = opendir("/home/w1155959/ecs150/");

  DIR *midir;
  struct dirent* info_archivo;
  struct stat fileStat;
  char fullpath[256];

  string test = currentDir;
  if ((midir=opendir(currentDir)) == NULL)
  {
    //perror("Error in opendir");
    string error = "Failed to open directory \"";
    int errorLen = error.length();
    write(1,const_cast<char *>(error.c_str()),errorLen);
    strcpy(fullpath, const_cast<char *>(test.c_str()));
    strcat(fullpath,"/");
    error = fullpath;
    write(1,const_cast<char *>((error.c_str())),error.length());
    write(1,"\"\n",2);
    exit(-1);
  }
  while ((info_archivo = readdir(midir)) != 0)
  {
    strcpy (fullpath, const_cast<char *>(test.c_str()));
    strcat (fullpath, "/");
    strcat (fullpath, info_archivo->d_name);
    //stat takes information on the fullpath and sticks it in fileStat, and returns 0 if successful
    if (!stat(fullpath, &fileStat))
    {
      if(S_ISDIR(fileStat.st_mode)){
        write(1,"d",1);
      }
      else write(1,"-",1);
      
      if(fileStat.st_mode & S_IRUSR){
        write(1,"r",1);
      }
      else write(1,"-",1);

      if(fileStat.st_mode & S_IWUSR){
        write(1,"w",1);
      }
      else write(1,"-",1);
      
      if(fileStat.st_mode & S_IXUSR){
        write(1,"x",1);
      }
      else write(1,"-",1);
      
      if(fileStat.st_mode & S_IRGRP){
        write(1,"r",1);
      }
      else write(1,"-",1);
      
      if(fileStat.st_mode & S_IWGRP){
        write(1,"w",1);
      }
      else write(1,"-",1);
      
      if(fileStat.st_mode & S_IXGRP){
        write(1,"x",1);
      }
      else write(1,"-",1);
      
      if(fileStat.st_mode & S_IROTH){
        write(1,"r",1);
      }
      else write(1,"-",1);
      
      if(fileStat.st_mode & S_IWOTH){
        write(1,"w",1);
      }
      else write(1,"-",1);
      
      if(fileStat.st_mode & S_IXOTH){
        write(1,"x",1);
      }
      else write(1,"-",1);

      //example print statements from stack overflow
      //printf((S_ISDIR(fileStat.st_mode))  ? "d" : "-");
      //printf((fileStat.st_mode & S_IRUSR) ? "r" : "-");
      //printf((fileStat.st_mode & S_IWUSR) ? "w" : "-");
      //printf((fileStat.st_mode & S_IXUSR) ? "x" : "-");
      //printf((fileStat.st_mode & S_IRGRP) ? "r" : "-");
      //printf((fileStat.st_mode & S_IWGRP) ? "w" : "-");
      //printf((fileStat.st_mode & S_IXGRP) ? "x" : "-");
      //printf((fileStat.st_mode & S_IROTH) ? "r" : "-");
      //printf((fileStat.st_mode & S_IWOTH) ? "w" : "-");
      //printf((fileStat.st_mode & S_IXOTH) ? "x" : "-");
    }
    //write(1, info_archivo->d_name);
    string s = info_archivo->d_name;
    write(1," ",1);
    write(1,s.c_str(),s.length());
    write(1,"\n",1);
    //printf (" %s", info_archivo->d_name);
    //printf("\n");
  }
  closedir(midir);
  /*while((myfile = readdir(mydir)) != NULL){
    printf("%s ", myfile->d_name);
    strcpy(buf, currentDir);
    strcat(buf, "/");
    strcat(buf, myfile->d_name);
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
  //stat(buf,&mystat);
  //printf("%s\n", myfile->d_name);

  }*/
}



int pwd(){
  /* Immediately get the current working directory, this code is pulled from http://stackoverflow.com/questions/143174/how-do-i-get-the-directory-that-a-program-is-running-from on 3/29/2016 by original user computinglife and edited by Polar*/
  char cCurrentPath[FILENAME_MAX];
  if(!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath))){
    return errno;}
  cout << "Current working directory: " << cCurrentPath << endl;
  return 0;

}

string getPwd(){
  char cCurrentPath[FILENAME_MAX];
  //ERROR CATCH MISSING FROM HERE, SEE ABOVE pwd();
  GetCurrentDir(cCurrentPath, sizeof(cCurrentPath));
  string test = cCurrentPath;
  return test;
}

int main(){

  vector<command> cmds;
  vector<string> tkns; //tokens
	
  


  struct termios termModeAttributes;
  //opengroup.org 
  char RXChar;
  string inputStr;
  //STDIN_FILENO is defined as 0 in unistd.h
  start_shell(STDIN_FILENO,&termModeAttributes);
  //char tesat[128];
  //scanf("%127s", tesat);
  //write(STDIN_FILENO, tesat, strlen(tesat));
  while(1){
    read(STDIN_FILENO, &RXChar, 1); // number of bytes read
    if(0x04 == RXChar){
      break;
    }
    //check for up & down arrow
    //also check for left and right arrows, in which case do nothing
    if(0x1B == RXChar){
      read(STDIN_FILENO, &RXChar, 1);
      if(0x5B == RXChar){
        read(STDIN_FILENO, &RXChar, 1);
        if(0x41 == RXChar){
          cout << endl << "UP" << endl;
        }
        if(0x42 == RXChar){
          cout << "DOWN" << endl;
        }
        if(0x44 == RXChar || 0x43 == RXChar){
          //do nothing
          //cout << "RIGHT/LEFT" << endl;
        }
      }
    }
    else{
      //if the enter key hasn't been pressed, add RXChar to input str
      if(0x0A != RXChar){
        inputStr += RXChar;
      }
    }
    if(0x0A == RXChar){
      //put each token in tkns[i]
      //tokenize(str, tkns); 
      char* inputString = new char[inputStr.size()+1];
      copy(inputStr.begin(), inputStr.end(),inputString);
      inputString[inputStr.size()] = '\0';
      tokenize(inputString, tkns);

      if ( inputStr == "exit" ){
        break;
      }
      else if (inputStr == "pwd"){
        pwd();
      }
      // need to trim leading whitespace
      //else if (inputStr[0] == 'l' && inputStr[1] == 's' && inputStr.length() > 2){
      //  ls("");
      //}
      else{
        //now split it into commands
        getCmds(tkns, cmds);
        loop_pipe(cmds);
      }
      //else{
      //  cout << "Failed to execute " + inputStr << endl;
      //}
      inputStr = "";
      delete[] inputString;
      cmds.clear();
      tkns.clear();
    }

  }
  reset_shell(STDIN_FILENO,&termModeAttributes);
  return 0;
}
