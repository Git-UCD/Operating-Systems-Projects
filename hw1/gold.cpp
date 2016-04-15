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

void ff(vector<command> cmd);
void ff( string rootDir , string findfile );
void pwd();
void ls(string directory);
void cd(string directory);
string getPwd();

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

string cleanStr(string str){
  string out = "";
  for (int i = 0;i < str.length(); i ++){
    if (str[i] != ' '){
      out += str[i];
    }
  }
  return out;
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
    if(cmds[i].cmd != "|" && cmds[i].cmd != "<" && cmds[i].cmd != ">" && cmds[i].cmd != ">>") commands++;
    if(cmds[i].cmd == "<" || cmds[i].cmd == ">" || cmds[i].cmd == ">>"){
      commands--;
    }
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
  int pipes = numOfPipes(cmds);
  int child = 0;
  int outflag = 0;
  int inflag = 0;

 /* 
     for(int i = 0; i < cmds.size(); i++){
     cout << cmds[i].cmd << endl;
     }
     cout << "cmds.size(): " << cmds.size() << endl;
     cout << "commands: " << commands << endl;
     cout << "redirects: " << redirects << endl;
     cout << "pipes: " << pipes << endl << endl << endl; 
   */  

  for(int i = 0; i < cmds.size();){
    int outflag = 0;
    int inflag = 0;
    int in, out;
    pipe(p);
    pid = fork();
    child++;
    if(pid == 0){
      //cout << "child: " << child << endl;
      //cout << "i: " << i << endl;
      dup2(fd_in,0);
      if((i+1) < cmds.size() && cmds[i+1].cmd == ">"){
        out = open(const_cast<char *>(cmds[i+2].cmd.c_str()), O_WRONLY |                            O_TRUNC | O_EXCL | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP                          | S_IWUSR);
        outflag = 1;
      }
      if((i+1) < cmds.size() && cmds[i+1].cmd == "<"){
        in = open(const_cast<char *>(cmds[i+2].cmd.c_str()), O_RDONLY);
        if(in == -1){
          write(1, "File \"",6);
          write(1, (cmds[i+2].cmd).c_str(), cmds[i+2].cmd.length());
          write(1, "\" does not exist!",17);
          exit(EXIT_FAILURE);
          //there is no \n in ashell
          //write(1, "\n",1);
        }
        inflag = 1;
        if((i+3) < cmds.size() && cmds[i+3].cmd == ">"){
          out = open(const_cast<char *>(cmds[i+4].cmd.c_str()), O_WRONLY | O_TRUNC | O_EXCL | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
          outflag = 1;

        }
      }


      if(child != (commands)){
        // replace standard out with the childs output exec if it's not the last            child
        dup2(p[WRITE_END], STDOUT_FILENO);
        if(outflag){
          //further replace standard out to file
          dup2(out, STDOUT_FILENO);
          close(out);
        }
        if(inflag){
          dup2(in, STDIN_FILENO);
          close(in);
        }
      }
      else{
        if(outflag){
          //replace the file with childs output
          //replace standard out with the file
          dup2(out, STDOUT_FILENO);
          close(out);
        }
        if(inflag){
          dup2(in, STDIN_FILENO);
          close(in);
        }
      }
      close(p[READ_END]);

      int argc = numOptions(cmds,i)+1;
      char* args[argc];
      args[0] = const_cast<char *>((cmds[i].cmd).c_str());
      for(int x = 1; x < argc; x++){
        args[x] = const_cast<char *>((cmds[i].opt).c_str());
      }
      args[argc] = NULL;
      if(cmds[i].cmd == "ls"){
        ls(cmds[i].opt);
      }
      else if(cmds[i].cmd == "pwd"){
        pwd();
      }
      else if(cmds[i].cmd == "ff"){
        //cout << "cmdS[0].cmd: " <<cmds[0].cmd << endl;
        //cout << "cmdS[0].opt: " << cmds[0].opt << endl;
        //int test;
        //cout << cmds[i].cmd << endl;
        //cout << cmds[i].opt << endl;
        vector<command> cmdX;
        vector<string> tkns; //tokens
        char* inputStringChar = const_cast<char *>((cmds[0].opt).c_str());
        tokenize(inputStringChar, tkns);
        getCmds(tkns, cmdX);
        //cout << "cmd[0].cmd: " << cmdX[0].cmd << endl; //gold.cpp
        //cout << "cmd[0].opt: " << cmdX[0].opt << endl; //ecs150
        if(cmds[i].opt.empty()){
          string msg = "ff command requires a filename!\n";
          write(STDOUT_FILENO,msg.c_str(),msg.length());
          exit(EXIT_FAILURE);
        }
        else{
          ff(cmdX[0].opt,cmdX[0].cmd);
        }
        //ff(getPwd(),"gold.cpp");
        //ff(getPwd(),cmds[i].opt);
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
      if(i < cmds.size()) i++;
      while(i < cmds.size() && cmds[i].cmd == "|"){
        i++;
      }
      if(i < cmds.size() && cmds[i].cmd == ">"){
        //cout << "OUTFLAG" << endl;
        //skip over the > and filename
        i = i+2;
        outflag = 0;
      }
      if(i < cmds.size() && cmds[i].cmd == "<"){
        i = i+2;
        inflag = 0;
        if(i < cmds.size() && cmds[i].cmd == ">"){
          i = i+2;
          outflag = 0;
        }
      }
    }
  }
}
//////////////// END PIPE.CPP


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


void purifyString(vector<command>& cmds){
  //cout << cmds.size() << endl;
  //cout << cmds[0].cmd << endl;
  //cout << cmds[1].cmd << endl;
  //cout << cmds[2].cmd << endl;
  for(int i = 0; i < cmds.size(); i++){
    if(i < cmds.size() && cmds[i].cmd == ">"){
      if((i+2) < cmds.size() && cmds[(i+2)].cmd == ">"){
        //cout << "Erase 1" << endl;
        //cout << "erasing: " << i << endl;
        cmds.erase(cmds.begin()+i);
        cmds.erase(cmds.begin()+i);
        i = i - 2;
      }
      if((i+2) < cmds.size() && cmds[(i+2)].cmd == "|"){
        cmds.erase(cmds.begin()+i);
        cmds.erase(cmds.begin()+i);
        i = i - 2;
      }
    }
  }
  //cout << "PRINTING" << endl;
  //for(int i = 0; i < cmds.size(); i++){
  //  cout << cmds[i].cmd << endl;
  //}

  //cout << "cmds.size(): " << cmds.size() << endl;
  //cout << endl << endl << endl;

  
}

void cd(vector<command> cmds){
  char *currentDir;
  string currentDirectory;
  if((cmds[0].opt).empty()){
    currentDirectory = getenv("HOME");
    currentDir = const_cast<char *>(currentDirectory.c_str());
  }
  else{
    currentDir = const_cast<char *>((cmds[0].opt).c_str());
  }

  chdir(currentDir);
}
void pwd(){
  /* Immediately get the current working directory, this code is pulled from http://stackoverflow.com/questions/143174/how-do-i-get-the-directory-that-a-program-is-running-from on 3/29/2016 by original user computinglife and edited by Polar*/
  char cCurrentPath[FILENAME_MAX];
  if(!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath))){
    //perror("error");
  }
  write(STDOUT_FILENO, cCurrentPath, strlen(cCurrentPath));
  write(STDOUT_FILENO,"\n",1);
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
  
}

void ff( string rootDir , string findfile ){
   //cout << endl << "rootdir: " << rootDir << endl;
   //cout << "findfile: " << findfile << endl;
   char *curreDir;
   if(rootDir.empty()){
   //string currentDirectory = getPwd();
   string currentDirectory = ".";
   curreDir = const_cast<char *>(currentDirectory.c_str());
   }
   else{
   curreDir = const_cast<char *>((rootDir).c_str());
   }
  //cout << "TEST:" << findfile << endl; 
  if (findfile.empty()){
    string msg = "ff command requires a filename!\n";
    write(STDOUT_FILENO,msg.c_str(),msg.length());
    return;
  }
  //cout << "Current directory: " << curreDir << endl;
  vector< string >dirs;
  DIR *mydir;
  struct dirent *myfile;
  string filePath;

  const char* currentDir = curreDir;
  string currentDirString = currentDir;
  mydir = opendir(currentDir);

  while((myfile = readdir(mydir)) != NULL){
    string strFile(myfile->d_name,strlen(myfile->d_name));
    // is a director
    if ( myfile->d_type == DT_DIR && strFile != "." && strFile != ".."  ){
      //cout << "PUSHING: " << strFile << endl;
      dirs.push_back(strFile);
    }
    else{
      // is a file
      if ( strFile == findfile){
        filePath = currentDirString + "/" + strFile;
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
  //cout << "dirs size: " << dirs.size() << endl;
  for(int i = 0;i < dirs.size(); i++){
    string newroot = currentDirString + "/" + dirs[i];
    ff(newroot,findfile);
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
  int maxHistory = 10;

  vector<command> cmds;
  vector<string> tkns; //tokens

  list<string>::iterator iter;
  list<string>::iterator iterUp;
  list<string>::iterator iterDwn;
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
          //auto nx  = std::next(iterUp,2);  
          if ( iterUp != history.end() && !history.empty() ){
            // clear
            for(int i = 0; i < inputLen; i++){
              write(STDOUT_FILENO,"\b \b", 3);
            }
            inputLen = 0;
            iterDwn = iterUp;
            // iterUp++;
            write(STDOUT_FILENO,(*iterUp).c_str(),(*iterUp).length());

            inputStr = *iterUp;
            inputLen += (*iterUp).length();
            iterUp++;
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

          if (iterDwn != history.begin() && !history.empty() ){

            for(int i = 0; i < inputLen; i++){
              write(STDOUT_FILENO,"\b \b", 3);
            }
            inputLen = 0;
            iterUp = iterDwn;
            iterDwn--;

            write(STDOUT_FILENO,(*iterDwn).c_str(),(*iterDwn).length());
            inputStr = *iterDwn;
            inputLen += (*iterDwn).length(); 
          }
          else{
            // downward end of the list
            // bellsound
            // cout << "bellsound down" << endl;
            write(STDOUT_FILENO,"\a",1);
            iterUp = history.begin();
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
      if (history.size() > maxHistory){
        history.pop_back();
      }

      if ( !history.empty() ){

        iterUp = history.begin();
      }
      //put each token in tkns[i]
      //tokenize(str, tkns); 
      char* inputStringChar = new char[inputStr.size()+1];
      copy(inputStr.begin(), inputStr.end(),inputStringChar);
      inputStringChar[inputStr.size()] = '\0';
      tokenize(inputStringChar, tkns);
      getCmds(tkns, cmds);
      purifyString(cmds);

      inputStr = cleanStr(inputStr);
      if(inputStr.empty()){
      }
      else if ( inputStr == "exit" ){
        break;
      }
      // need to trim leading whitespace
      //else if (inputStr[0] == 'l' && inputStr[1] == 's' && inputStr.length() > 2){
      //  ls("");
      //}
      else if(cmds[0].cmd == "cd"){
        cd(cmds);
      }
      //else if(inputStr == "ff"){
      //  ff(getPwd(),"gold.cpp");
      //}
      else if(cmds[0].cmd == ">" && numOfCommands(cmds) == 0 && numOfRedirects(cmds) == 1 && numOfPipes(cmds) == 0){
        int commands = numOfCommands(cmds); // number of commands to run
        int redirects = numOfRedirects(cmds);
        int pipes = numOfPipes(cmds);
        //cout << commands << endl << redirects << endl << pipes << endl;
        int out = open(const_cast<char *>(cmds[1].cmd.c_str()), O_WRONLY |                            O_TRUNC | O_EXCL | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP                          | S_IWUSR);
        close(out);

      }
      else if(cmds[0].cmd == "<" && numOfCommands(cmds) == 0 && numOfRedirects(cmds) == 1 && numOfPipes(cmds) == 0){
      int in = open(const_cast<char *>(cmds[1].cmd.c_str()), O_RDONLY);
      if(in == -1){
          write(1, "File \"",6);
          write(1, (cmds[1].cmd).c_str(), cmds[1].cmd.length());
          write(1, "\" does not exist!",17);
          exit(EXIT_FAILURE);
          //there is no \n in ashell
          //write(1, "\n",1);
        }

      }
      else{
        //now split it into commands
        //LS IS STILL HANDLED INTERNALLY
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
