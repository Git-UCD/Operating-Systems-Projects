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
//Contains all thread create to lookup
vector<TCB> threadList;

TVMThreadID curThreadID = NULL; // current operating thread


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
	TVMMainEntry mainEntry =  VMLoadModule(argv[0]);
	if(mainEntry == NULL){
		return VM_STATUS_FAILURE;
	}
	// cout << "TIMER:" << TIMER << endl;
	int flag = false;

	TMachineAlarmCallback callback = callbackAlarm;
	MachineRequestAlarm(tickms*1000,callback,&flag);
	mainEntry(argc,argv);
	// cout << "TIMER:" << TIMER << endl;

	//cout << "flag:" << flag << endl;
	while(!flag);
	TCB tstartBlk;




	
	MachineTerminate();
	return VM_STATUS_SUCCESS; 
}
TVMStatus VMTickMS(int *tickmsref){
	if(tickmsref == NULL){
		return VM_STATUS_ERROR_INVALID_PARAMETER;
	}
	//tickmsref = 

	return VM_STATUS_SUCCESS; 
}

TVMStatus VMTickCount(TVMTickRef tickref){
	if(tickref == NULL){
		return VM_STATUS_ERROR_INVALID_PARAMETER;
	}

	return VM_STATUS_SUCCESS; 
}

void threadWrapper(){

}

TVMStatus VMThreadCreate(TVMThreadEntry entry, void *param, TVMMemorySize memsize, TVMThreadPriority prio, TVMThreadIDRef tid){
	if(entry == NULL || tid == NULL ){
		return VM_STATUS_FAILURE;
	}
	TCB threadB;
	// MachineContextCreate(SMachineContextRef mcntxref, void (*entry)(void *), void *param, void *stackaddr, size_t stacksize);
	threadB.mmSize = memsize;
	threadB.priority = prio;
	threadB.id = *tid;
	threadList.push_back(threadB);
	
return VM_STATUS_SUCCESS; 
}

TVMStatus VMThreadDelete(TVMThreadID thread){
	for(int i = 0;i < threadList.size();i++){
        if(threadList[i].id == thread){
			// delete thread
      }
   }
}


TVMStatus VMThreadActivate(TVMThreadID thread){
	bool found = false;
	// find  thread with matching id given
	for(int i = 0;i < threadList.size();i++){
        if(threadList[i].id == thread){

        	found = true;
        	if ( threadList[i].state == VM_THREAD_STATE_DEAD){
        		return VM_STATUS_ERROR_INVALID_STATE;
        	}
        	// activate thread
		}
    }

    if(!found){
    	return VM_STATUS_ERROR_INVALID_ID;
    }

    return VM_STATUS_SUCCESS;
}

TVMStatus VMThreadTerminate(TVMThreadID thread){
	bool found = false;
	// find  thread with matching id given
	for(int i = 0;i < threadList.size();i++){
        if(threadList[i].id == thread){
        	// thread enters the dead state
        	found = true;
        	if(threadList[i].state == VM_THREAD_STATE_DEAD ){
        		return VM_STATUS_ERROR_INVALID_STATE;
        	}
		}
    }

    if(!found){
    	return VM_STATUS_ERROR_INVALID_ID;
    }
    return VM_STATUS_SUCCESS;
}

TVMStatus VMThreadID(TVMThreadIDRef threadref){
	if(threadref == NULL){
		return VM_STATUS_ERROR_INVALID_PARAMETER;
	}
	//threadref = curThreadID;
	return VM_STATUS_SUCCESS;
}

TVMStatus VMThreadState(TVMThreadID thread, TVMThreadStateRef stateref){
	if(stateref == NULL){
		return VM_STATUS_ERROR_INVALID_PARAMETER;
	}
	bool found = false;
	// find  thread with matching id given
	for(int i = 0;i < threadList.size();i++){
        if(threadList[i].id == thread){
        	// delete thread
        	found = true;
		}
    }

    if(!found){
    	return VM_STATUS_ERROR_INVALID_ID;
    }
    return VM_STATUS_SUCCESS;	
}



TVMStatus VMThreadSleep(TVMTick tick){
	if (tick == VM_TIMEOUT_INFINITE){
		return VM_STATUS_ERROR_INVALID_PARAMETER;
	}
	if (tick == VM_TIMEOUT_IMMEDIATE){
		//current process yields the remainder of its processing quantum
		// to the next ready process of equla priority
	}
	else{
	// put current thread to sleep
		TIMER = tick;
		while(TIMER != 0){

		}
	}

	return VM_STATUS_SUCCESS;
}





void fileWCallback(void* a,int result){
 
}
 
TVMStatus VMFileWrite(int filedescriptor, void *data, int *length){
    // write(filedescriptor,data,*length);
    TMachineFileCallback myfilecallback = fileWCallback;
    int calldata = NULL;
    MachineFileWrite(filedescriptor, data, *length, myfilecallback, &calldata);
    cout << "write: " << calldata << endl;

    
    return VM_STATUS_SUCCESS;
}
 
void fileOpenCallback(void* calldata, int result){
    result = 9;
}
 
TVMStatus VMFileOpen(const char *filename, int flags, int mode, int *filedescriptor){
	TMachineFileCallback fOpenCallback = fileOpenCallback;
	int callData;
	MachineFileOpen(filename, flags,  mode, fOpenCallback, &callData );
	*filedescriptor = callData;

 return VM_STATUS_SUCCESS;
 
}
void fileCloseCallback(void* calldata,int result){

}
TVMStatus VMFileClose(int filedescriptor){
	TMachineFileCallback fCloseCallback = fileCloseCallback;
	int results;
	MachineFileClose(filedescriptor, fCloseCallback, &results);
	return VM_STATUS_SUCCESS;
}  
void fileSeekCallback(void* calldata,int result){

}

TVMStatus VMFileSeek(int filedescriptor, int offset, int whence, int *newoffset){
	TMachineFileCallback fSeekCallback = fileSeekCallback;
	int results;
	MachineFileSeek( filedescriptor, offset, whence, fSeekCallback, newoffset);

    return VM_STATUS_SUCCESS;
}

void fileReadCallback(void* calldata,int result){
	
}


TVMStatus VMFileRead(int filedescriptor, void *data, int *length){
	TMachineFileCallback fReadCallback = fileReadCallback;
	int results;
	MachineFileRead(filedescriptor, data, *length, fReadCallback, &results);
	return VM_STATUS_SUCCESS;
}

 
 

}
