#include "VirtualMachine.h"
#include "Machine.h"
#include <iostream>
#include <queue>
#include <vector>
using namespace std;
extern "C" {

volatile int TIMER;
static const int HIGH = 3;
static const int MED = 2;
static const int LOW = 1;

class TCB{
  public:
	TVMThreadID id;
	TVMThreadPriority priority;
	TVMThreadState state;
	TVMMemorySize mmSize;
	TVMStatus status;
	TVMTick vmTick;
	
};

//PRIORITY QUEUE SETUP
//WILL RETURN HIGH PRIORITY > MED PRIORITY > LOW PRIORITY
struct LessThanByPriority{
  bool operator()(const TCB& lhs, const TCB& rhs) const{
    return lhs.priority < rhs.priority;
  }
};
typedef priority_queue<TCB, vector<TCB>, LessThanByPriority> pq;
// END PRIORITY QUEUE SETUP



TVMMainEntry VMLoadModule(const char *module);
TVMStatus VMFilePrint(int filedescriptor, const char *format, ...);

void  callbackAlarm( void* t){
	
	TIMER--;
    
}

TVMStatus VMStart(int tickms, int argc, char *argv[]){
	MachineInitialize();
	// request time
	// int time = 0;
	int flag;
	TIMER = 100;
	TVMMainEntry mainEntry =  VMLoadModule(argv[0]);
	if(mainEntry == NULL){
		return VM_STATUS_FAILURE;
	}
	cout << "TIMER:" << TIMER << endl;
  cout << "tickms: " << tickms << endl;
	TMachineAlarmCallback callback = callbackAlarm;
	MachineRequestAlarm(tickms*1000,callback,&flag);
	mainEntry(argc,argv);
	cout << "TIMER:" << TIMER << endl;

	cout << "flag:" << flag << endl;

	TCB tstartBlk;




	
	MachineTerminate();
	return VM_STATUS_SUCCESS; 
}

TVMStatus VMThreadSleep(TVMTick tick){
	// TIMER = tick;
	while(TIMER != 0){

	}

return VM_STATUS_SUCCESS; 	
}

TVMStatus VMThreadCreate(TVMThreadEntry entry, void *param, TVMMemorySize memsize, TVMThreadPriority prio, TVMThreadIDRef tid){
	TCB threadB;
return VM_STATUS_SUCCESS; 
}


// TVMStatus VMTickMS(int *tickmsref){

// }

// TVMStatus VMTickCount(TVMTickRef tickref){

// }

TVMStatus VMFileWrite(int filedescriptor, void *data, int *length){
	write(filedescriptor,data,*length);
//	TMachineFileCallback mycall = 
//	MachineFileWrite(filedescriptor, *data, *length, mycallback, void *calldata);
	return VM_STATUS_SUCCESS;
}
}
