#include <vector>
#include <string>
#include <iostream>
using namespace std;

class command{
  public:
      string cmd;
      string opt;
};

int main(){
  vector<command> cmds;
  string a = "a";
  string b = "b";
  command *newcmd = new command();
  newcmd->cmd = a;
  newcmd->opt = b;

  cout << newcmd->cmd << endl << newcmd->opt << endl;
  cmds.push_back(*newcmd);
    
  string c = "c";
  string d = "d";
  command *newcmd2 = new command();
  newcmd2->cmd = c;
  newcmd2->opt = d;
  cmds.push_back(*newcmd2);
  cout << cmds[0].cmd << endl << cmds[0].opt << endl;
  cout << cmds[0].cmd << endl << cmds[0].opt << endl;
  cout << cmds[1].cmd << endl << cmds[1].opt << endl;
  cout << cmds[1].cmd << endl << cmds[1].opt << endl;
}
