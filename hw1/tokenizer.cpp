#include <stdio.h>
#include <string.h>
#include <string>
#include <sstream>
#include <vector>
#include <iostream>
using namespace std;

class command{
  public:
    string cmd;
    string opt;
};


int main(){
  // < input
  // > output
  // >> append
  //char str[] = "ls -l | grep pipe < file.txt";
  //char str[] = "ls -l > file.txt"; //out ls -l to file.txt
  //vector<command> cmds;
  char str[] = "ps aux | grep conky | grep -v grep | awk '{print $2}' | xargs kill";
  //char str[] = "cat filename | less";

  char *s;
  string sstring;
  s = strtok(str," ");
  
  while(s != NULL){
    sstring = s;
    //cout << "str: " << s  << endl;
    //ls
    if(sstring == "ls"){
      //command *newcmd = new command();
      //newcmd.cmd = sstring;
      s = strtok(NULL, " ");
      sstring = s;
      //cout << "char str: " << s << endl ;
      if(sstring[0] == '-'){
        //newcmd.opt = sstring;
        string ls = "ls ";
        ls.append(s);
        cout << "execv(ls," << ls << ")"<<endl;
      }
      //cmds.push_back(newcmd);
    }

    else{ //cmd not ls
      string cmd = s;
      string options = cmd;
      options.append(" ");
      int redirectlessthan = 0;
      int redirectmorethan = 0;
      int redirectappendmorethan = 0;
      int redirect = 0;
      int addingOptions = 0;
      //cout << "CMD: " << cmd << " options: " << options << "-" << endl;
      //this gets options
      while(s != NULL && sstring != "|" && sstring != "<" && sstring != ">" && sstring != " >>" && redirect != 1){
        redirect = redirectlessthan = redirectmorethan = redirectappendmorethan = 0;
        s = strtok(NULL, " ");
        if(s != NULL){
          sstring = s;
        }
        cout << "Sstring: " << sstring << "." <<  endl;
        if(s != NULL && sstring != "|" && sstring != ">" && sstring != "<" && sstring != ">>"){
          sstring = s;
          if(addingOptions){
            options.append(" ");
          }
          if(sstring != "<" && sstring != ">" && sstring != ">>"){
            options.append(sstring);
            addingOptions = 1;
            //cout << "execv(" << cmd << ", " << options << ")" <<endl;
          }
        }

      }
      if(sstring == "<"){
        redirect = 1;
        redirectlessthan = 1;
        cout << "Perform <" << endl;
        cout << "exec " << options << " < ";
        s = strtok(NULL, " ");
        if(s!= NULL){
          sstring = s;
        }
        cout << sstring << endl;

      }
      if(sstring == ">"){
        redirect = 1;
        redirectmorethan = 1;
        cout << "Perform >" << endl;
        cout << "redirect >" ;
        s = strtok(NULL, " ");
        if(s!= NULL){
          sstring = s;
        }
        cout << sstring << endl;
      }
      if(sstring == ">>"){
        redirect = 1;
        redirectappendmorethan = 1;
        cout << "Perform >>" << endl;
        cout << "exec " << options << " >> " ;
        s = strtok(NULL, " ");
        if(s != NULL){
          sstring = s;
        }
        cout << sstring << endl;
      }
      if(redirect != 1) cout << "execv(" << cmd << " , " << options << ")" << endl;
    }

    //suck up |
    s = strtok(NULL, " ");
    if(s != NULL) sstring = s;
    if(sstring == "|"){
      s = strtok(NULL, " ");
      sstring = s;
    }
    //
  }
  return 0;
}
