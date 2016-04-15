#include <iostream>
#include <vector>
using namespace std;

class cmd{
  public:
    int a;
};

int main(){
  vector<cmd> cmds;
  for(int i = 0; i < 10; i++){
    cmd *newCmd = new cmd();
    newCmd->a = i;
    cmds.push_back(*newCmd);
  }
  for(int i = 0; i < 10; i++){
    cout << cmds[i].a << endl;
  }
}
