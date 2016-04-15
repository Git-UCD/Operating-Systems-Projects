#include <unistd.h>
#include <fcntl.h>
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
#include <list>
#include <iterator>
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
  int reDirects = 0;
  for(int i = 0; i < cmds.size(); i++){
    if(cmds[i].cmd == "<" | cmds[i].cmd == ">" | cmds[i].cmd == ">>"){
      reDirects++;
      //commands--;
    }
    if(cmds[i].cmd != "|" && cmds[i].cmd != "<" && cmds[i].cmd != ">" && cmds[i].cmd != ">>") commands++;
  }
  return commands;
}




int numOfPipes(vector<command> cmds){
  int reDirects = 0;
  for(int i = 0; i < cmds.size(); i++){
    if(cmds[i].cmd == "|"){
      reDirects++;
    }
  }
  return reDirects;
}
int numOfRedirects(vector<command> cmds){
  int reDirects = 0;
  for(int i = 0; i < cmds.size(); i++){
    if(cmds[i].cmd == "<" | cmds[i].cmd == ">" | cmds[i].cmd == ">>"){
      reDirects++;
    }
  }
  return reDirects;
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
  int redirects = numOfRedirects(cmds);
  int child = 0;
  //cout << commands << endl;
  //cout << redirects << endl;
  // print everything in cmds
  //cout << cmds.size() << endl;
  //for(int i = 0; i < cmds.size(); i++){
  //  cout << cmds[i].cmd << endl;
  //}

  for(int i = 0; i < (cmds.size() - redirects);){
    //cout << "fd_in: " << fd_in << endl;
    int outflag = 0;
    int inflag = 0;
    int in, out;
    pipe(p);
    pid = fork();
    child++;
    //while(cmds[i].cmd == ">" || cmds[i].cmd == "<" || cmds[i].cmd == ">>"){
    //  i = i + 2;
    //  if(cmds[i].cmd == "|") i++;
    //}
    if(pid == 0){
      //cout << "In new child" << endl;
      //cout << "old fd: " << fd_in << " new fd: 0 (closing old fd and copying it to 0: stdin)" << endl;

      if((i+1) < commands && cmds[i+1].cmd == ">")
      {
        out = open(const_cast<char *>(cmds[i+2].cmd.c_str()), O_WRONLY | O_TRUNC | O_EXCL | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
        outflag = 1;
      }
      else if(i < commands-1 && cmds[i+1].cmd == "<"){
        in = open("shell.cpp", O_RDONLY);
        inflag = 1;
        if(cmds[i+3].cmd == ">"){
          out = open(const_cast<char *>(cmds[i+4].cmd.c_str()), O_WRONLY | O_TRUNC | O_EXCL | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
          outflag = 1;
        }
      }
      //cout << cmds.size() << " != " << i+1 << endl;
      if(i+1 != cmds.size()){
        dup2(p[WRITE_END],STDOUT_FILENO);
        if (inflag){
          dup2(in,0);
          close(in);

        }
        if (outflag){
          dup2(out,1);
          close(out);
        }
      }
      else {
        if (inflag){
          dup2(in, 0);
          close(in);
        }
        if (outflag){
          dup2(out,1);
          close(out);
        }
      }

      close(p[READ_END]);
      //if(child == 1){
      //using http://stackoverflow.com/questions/28712552/c-using-execvp-with-string-array to convert string to stupid execvp format
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
      exit(EXIT_FAILURE);
    }
    else{
      wait(NULL);
      close(p[WRITE_END]);
      fd_in = p[READ_END];
      if(outflag == 1){
        i = i+3;
      }
      else{
        i++;
      }
      while(i < commands && cmds[i].cmd == "|"){
        i++;
      }
    }
    outflag = 0;
    inflag = 0;
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
    exit(0);
  }
  // Save the terminal attributes
  tcgetattr(fd,savedattributes);

  // save terminal modes
  tcgetattr(fd,&TermAttributes);

  TermAttributes.c_lflag &= ~(ECHO); /*local modes*/ 
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

void ff(string rootDir, string findfile){
  if (findfile.length() == 0){
    string msg = "ff command requires a filename!\n";
    write(STDOUT_FILENO,msg.c_str(),msg.length());
    return;
  }
  vector< string >dirs;
  DIR *mydir;
  struct dirent *myfile;
  string filePath;

  const char* currentDir = rootDir.c_str();
  mydir = opendir(currentDir);

  while((myfile = readdir(mydir)) != NULL){
    string strFile(myfile->d_name,strlen(myfile->d_name));
    // is a director
    if ( myfile->d_type == DT_DIR && strFile != "." && strFile != ".."  ){
      dirs.push_back(strFile);
    }
    else{
      // is a file
      if ( strFile == findfile){
        filePath = rootDir + "/" + strFile;
        write(STDOUT_FILENO,filePath.c_str(),filePath.length());
        write(STDOUT_FILENO,"\n",1);
      }
    }

  }
  closedir(mydir);
  // base case
  if (dirs.size() == 0){
    return;
  }
  for(int i = 0;i < dirs.size(); i++){
    string newroot = rootDir + "/" + dirs[i];
    ff(newroot,findfile);
  }

}

int pwd(){
  /* Immediately get the current working directory, this code is pulled from http://stackoverflow.com/questions/143174/how-do-i-get-the-directory-that-a-program-is-running-from on 3/29/2016 by original user computinglife and edited by Polar*/
  char cCurrentPath[FILENAME_MAX];
  if(!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath))){
    perror("error");
    write(STDOUT_FILENO, cCurrentPath, strlen(cCurrentPath));
    write(STDOUT_FILENO,"\n",1);
    return 0;

  }
}

string getPwd(){
  char cCurrentPath[FILENAME_MAX];
  //ERROR CATCH MISSING FROM HERE, SEE ABOVE pwd();
  GetCurrentDir(cCurrentPath, sizeof(cCurrentPath));
  string test = cCurrentPath;
  return test;
}

int findChar(string str,char key){
  int count = 0;
  for(int i = 0; i < str.length(); i++){
    if ( str[i] ==  key){
      count++;
    }
  }
  return count;
}

void writeTermPwd(){
  string fullpath  = getPwd();
  fullpath = fullpath + "% ";
  string wPath;

  int countPath = findChar(fullpath,'/');
  // only write the fullpath if it contains two directories
  if (countPath < 3){
    wPath = fullpath;

  }else{
    // get the position of the first occurrence of the backslash
    int fps = fullpath.find("/");
    int lps = fullpath.rfind("/"); // first occurrence reverse
    string dir = fullpath.substr(lps,fullpath.length()); //
    wPath = "/..." + dir;
  }
  write(STDOUT_FILENO,wPath.c_str(),wPath.length() );
}



int main(){
  struct termios termModeAttributes;

  char RXChar;
  string inputStr;
  int inputLen = 0;
  list<string> history;

  vector<command> cmds;
  vector<string> tkns; //tokens

  list<string>::iterator iter;
  list<string>::iterator iterUp = iter;
  //STDIN_FILENO is defined as 0 in unistd.h
  start_shell(STDIN_FILENO,&termModeAttributes);
  writeTermPwd();
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
          // up
          // look at the next location
          auto nx  = std::next(iter,1);
          if ( nx != history.end() && !history.empty() ){
            // clear
            for(int i = 0; i < inputLen; i++){
              write(STDOUT_FILENO,"\b \b", 3);
            }
            inputLen = 0;
            iter++;
            write(STDOUT_FILENO,(*iter).c_str(),(*iter).length());

            inputStr = *iter;
            inputLen += (*iter).length();
          }
          else{
            // upward end of the list
            // bellsound
            write(STDOUT_FILENO,"\a",1);
          }

        }
        if(0x42 == RXChar){
          for(int i =0; i < inputLen; i++){
            write(STDOUT_FILENO,"\b \b", 3);
          }
          inputLen = 0;
          inputStr = "";

          if (iter != history.begin() && !history.empty() ){

            for(int i = 0; i < inputLen; i++){
              write(STDOUT_FILENO,"\b \b", 3);
            }
            inputLen = 0;
            iter--;
            write(STDOUT_FILENO,(*iter).c_str(),(*iter).length());
            inputStr = *iter;
            inputLen += (*iter).length();
          }
          else{
            // downward end of the list
            // bellsound
            // cout << "bellsound down" << endl;
            write(STDOUT_FILENO,"\a",1);
          }

        }
        if(0x44 == RXChar || 0x43 == RXChar){
          //do nothing
          //cout << "RIGHT/LEFT" << endl;
        }
      }
    }
    else{
      if(0x7F == RXChar){
        if(inputLen>0){
          write(STDOUT_FILENO, "\b \b", 3);
          inputLen--;
          inputStr.pop_back();
        } 
      }      
      else{
        write(STDOUT_FILENO, &RXChar, 1);
        if(RXChar!= 0x0A){
          inputLen++;
          inputStr += RXChar;
        }
      }
    }

    if(0x0A == RXChar){
      inputLen = 0;
      history.push_front(inputStr);
      if(!history.empty()){
        iter = history.begin();
      }
      //put each token in tkns[i]
      //tokenize(str, tkns); 
      char* inputStringChar = new char[inputStr.size()+1];
      copy(inputStr.begin(), inputStr.end(),inputStringChar);
      inputStringChar[inputStr.size()] = '\0';
      tokenize(inputStringChar, tkns);

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
      else if(inputStr == "ff"){
        ff(getPwd(),"gold.cpp");
      }
      else{
        //now split it into commands
        getCmds(tkns, cmds);
        loop_pipe(cmds);
      }
      //else{
      //  cout << "Failed to execute " + inputStr << endl;
      //}
      writeTermPwd();
      inputStr = "";
      delete[] inputStringChar;
      cmds.clear();
      tkns.clear();
    }

  }
  reset_shell(STDIN_FILENO,&termModeAttributes);
  return 0;
}
