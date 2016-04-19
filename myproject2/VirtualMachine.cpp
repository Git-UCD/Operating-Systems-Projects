#include "VirtualMachine.h"
#include "Machine.h"
#include <iostream>
#include <queue>
using namespace std;

volatile int TIMER;

class TCB{
	//state
	TVMStatus status;
	
	TVMTick vmTick;
	//file

	//MM
	TVMMemorySize mmSize;

	TVMThreadID id;
	
	TVMThreadState state;

	TVMThreadPriority priority;
	
};

queue<TCB> threadBlks;

extern "C" {


TVMMainEntry VMLoadModule(const char *module);
TVMStatus VMFilePrint(int filedescriptor, const char *format, ...);

void  callbackAlarm( void* t){
	
	TIMER = 0;
    
}

TVMStatus VMStart(int tickms, int argc, char *argv[]){
	MachineInitialize();
	// request time
	// int time = 0;
	int flag;
	TIMER = 50;
	TVMMainEntry mainEntry =  VMLoadModule(argv[0]);
	if(mainEntry == NULL){
		return VM_STATUS_FAILURE;
	}
	cout << "TIMER:" << TIMER << endl;
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
