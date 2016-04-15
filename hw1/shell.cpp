
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
#define GetCurrentDir getcwd
#include <vector>
#include <list>
#include <iterator>


using namespace std;

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
  
  TermAttributes.c_lflag &= ~(ECHO); /*local modes*/ 
  TermAttributes.c_lflag &= ~(ICANON);
  TermAttributes.c_cc[VMIN] = 1;
  TermAttributes.c_cc[VTIME] = 0;
  tcsetattr(fd,TCSAFLUSH,&TermAttributes);
}

void ls(){
  DIR *mydir;
  struct dirent *myfile;
  struct stat mystat;
  char buf[512];
  string currentDirectory = getPwd();
  const char *currentDir = currentDirectory.c_str();
  //mydir = opendir("/home/w1155959/ecs150/");
  mydir = opendir(currentDir);
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


void ff( string rootDir , string findfile ){
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
  }
  write(STDOUT_FILENO,cCurrentPath,strlen(cCurrentPath));
  write(STDOUT_FILENO,"\n",1);
  return 0;

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
 
 
  list<string>::iterator iter;
  list<string>::iterator iterUp = iter;
  
  //STDIN_FILENO is defined as 0 in unistd.h
  start_shell(STDIN_FILENO,&termModeAttributes);

  writeTermPwd();

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
        } // end down arrow
        // if(0x44 == RXChar || 0x43 == RXChar){
        //   //do nothing
        //   //cout << "RIGHT/LEFT" << endl;
        // }
        // if(!(isprint(RXChar) ) ){
        //   cout << "nline \n";
        // }
      }
    } // arrows end
    else{
      // backspace delete
      if (0x7F == RXChar){
        if ( inputLen > 0){
          write(STDOUT_FILENO,"\b \b", 3);
          inputLen--;
          inputStr.pop_back();
          //inputLen--;
        }
       
      }
      else{
        // writing input to terminal window
        write(STDOUT_FILENO,&RXChar,1);
        if (RXChar != 0X0A){
          inputLen++;
          inputStr += RXChar;
        
        }
      
      }
    }
    // enter pressed 
    if(0x0A == RXChar){
      inputLen = 0;
     // cout << "inputStr: " << inputStr << "size:"<< inputStr.length() << endl;
      history.push_front(inputStr);
      if ( !history.empty() ){

        iter = history.begin();
      }

//////////////////////////
      if ( inputStr == "exit" ){
        break;
      }
      else if (inputStr == "pwd"){
        pwd();
      }
      // need to trim leading whitespace
      else if (inputStr[0] == 'l' && inputStr[1] == 's'){
       ls();
      }
      else if (inputStr == "ff"){
        ff(getPwd(),"");
      }
      else{
       
      }
      writeTermPwd();
       inputStr = "";
    }



  }
  reset_shell(STDIN_FILENO,&termModeAttributes);
  return 0;
}
