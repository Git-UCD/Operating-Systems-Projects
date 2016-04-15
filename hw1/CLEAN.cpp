#include <stdio.h>
#include <sstream>
#include <string.h>
#include <vector>
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

int main(){
  char str[] = "ps aux | grep conky | grep -v grep | awk '{print $2}' | xargs kill";
  //char str[] = "ls -l | grep pipe < file.txt";
  //char str[] = "ls -l > file.txt"; //out ls -l to file.txt
  vector<command> cmds;
  vector<string> tkns; //tokens

  //put each token in tkns[i]
  tokenize(str, tkns); 
  //now split it into commands
  getCmds(tkns, cmds);
  for(int i = 0; i < cmds.size(); i++){
    cout << cmds[i].cmd << " " << cmds[i].opt << endl;
  }

}
