#include <stdio.h>
#include <sstream>
#include <string.h>
#include <vector>
#include <unistd.h>
#include <cstdlib>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
using namespace std;

class command{
  public:
    string cmd;
    string opt;
};

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
        dup2(p[1],1);
      }
      //cout << "closing: " << p[0] << endl;
      close(p[0]);
      if(child == 1){
        //char* args[2];
        //args[0] = "ls";
        //args[1] = NULL;
        //cout << "CALLING" << endl;
        //cout << "Number: " << numOptions(cmds,i)+1 << endl;
        
        int argc = numOptions(cmds,i)+1;
        char* args[argc];
        args[0] = const_cast<char *>((cmds[i].cmd).c_str());
        for(int x = 1; x < argc; x++){
          args[x] = const_cast<char *>((cmds[i].opt).c_str());
        } 
        args[argc] = NULL;
        //cmdToExecString(cmds, i);
        execvp(args[0],args);
        //execvp(args[0], args);
      }
      else{
        char* args[2];
        args[0] = "wc";
        args[1] = NULL;
        execvp(args[0], args);
      }
      //execvp(*path, *arg);
      exit(EXIT_FAILURE);
    }
    else{
      wait(NULL);
      //cout << "In parent" << endl;
      //cout << "closing " << p[1] << endl;
      close(p[1]);
      //cout << "fd_in = " << p[0] << endl;
      fd_in = p[0];
      i++;
    }
  }
}

int main(){
  char str[] = "ls | wc";
  //char str[] = "ps aux | grep conky | grep -v grep | awk '{print $2}' | xargs kill";
  //char str[] = "ls -l | grep pipe";
  //char str[] = "ls -l | grep pipe < file.txt";
  //char str[] = "ls -l > file.txt"; //out ls -l to file.txt
  vector<command> cmds;
  vector<string> tkns; //tokens

  //put each token in tkns[i]
  tokenize(str, tkns); 
  //now split it into commands
  getCmds(tkns, cmds);
  //for(int i = 0; i < cmds.size(); i++){
  //  cout << cmds[i].cmd << " " << cmds[i].opt << endl;
  //}
  loop_pipe(cmds);

}
