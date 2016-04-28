#include "VirtualMachine.h"
#include "Machine.h"
#include <iostream>
#include <queue>
#include <vector>
#include <stdlib.h>
#include <iterator>
#include <string>

using namespace std;
extern "C" {

volatile int TIMER;
static const int HIGH = 3;
static const int MED = 2;
static const int LOW = 1;
TMachineSignalState sigstate;

volatile int threadCount = 1;



class TCB{
  public:
	TVMThreadID id;
  TVMThreadIDRef idref;
	TVMThreadPriority priority;
	TVMThreadState state;
	TVMMemorySize mmSize;
	TVMStatus status;
	TVMTick vmTick;
	TVMThreadEntry entryCB;
	void* param;
	SMachineContext context;
	
};

vector<TCB*> threadList;

TVMThreadID curThreadID; // current operating thread
TCB* curThread;



struct LessThanByPriority{
  bool operator()(const TCB* lhs, const TCB* rhs) const{
    return lhs->priority < rhs->priority;
  }
};
typedef priority_queue<TCB*, vector<TCB*>, LessThanByPriority> pq;


pq readyThreads;
vector<TCB*> sleepThreads;

void threadSchedule(){
   TCB* currentThread;
   for(unsigned int i = 0; i < threadList.size(); i++){
    if(curThreadID == threadList[i]->id){
      currentThread = threadList[i];
      //cout << "currentThread ID: " << currentThread->id << endl;
    }   
  }
  // cout << "current id: " << currentThread->id <<  endl;
  if(currentThread->state == VM_THREAD_STATE_RUNNING){
    currentThread->state = VM_THREAD_STATE_READY; 
  }
  if(currentThread->state == VM_THREAD_STATE_READY){
    //PUSH INTO PRIORITY QUEUE: readyThreads;
    readyThreads.push(currentThread);
  }

  

  SMachineContextRef oldContext = NULL;
  for (unsigned int i = 0; i < threadList.size();i++){
    if(curThreadID == threadList[i]->id){
      //cout << "Old context id: " << threadList[i]->id << endl;
      oldContext = &threadList[i]->context;
      if(threadList[i]->id == readyThreads.top()->id){
        readyThreads.pop();
        //push idle thread back onto ready threads
        if(threadList[i]->id == 2)
          readyThreads.push(threadList[i]);
      }
    }   
  }

  // cout << "switch " << endl;
  // if (readyThreads.empty()){
  // 	cout << "empty" << endl;
  // }
  //cout << "readyThreads.top.id: " << readyThreads.top()->id << endl;
  SMachineContextRef newContext = &readyThreads.top()->context;
  curThreadID = readyThreads.top()->id;
  //cout << "currentThread ID: " << currentThread->id << endl;
  currentThread->state = VM_THREAD_STATE_RUNNING;
  //cout << "Current Thread ID popped from readyThreads: " << curThreadID << endl;
  //if(!sleepThreads.empty()){
    //cout << "Something is in sleeping queue." << endl;
    //for(int i = 0; i < sleepThreads.size(); i++){
    //  cout << i << " is in sleeping Queue" << endl;
    //}
  //}
  MachineContextSwitch(oldContext,newContext);
  readyThreads.pop();
}

TCB* getTCB(TVMThreadID id){
	for (unsigned int i = 0;i < threadList.size();i++){
		if (id == threadList[i]->id){
			return threadList[i];
		}
	}
}

void  callbackAlarm( void* t){
	//cout << "callback" << endl;
	vector<TCB*>::iterator iter;

	for(iter = sleepThreads.begin(); iter != sleepThreads.end();iter++){
		//cout << "thread ticks: " << (*iter)->vmTick << endl;
		if ( (*iter)->vmTick == 0){
			//cout << "timeout:" << endl;
			(*iter)->state = VM_THREAD_STATE_READY; 
			readyThreads.push(*iter);
			sleepThreads.erase(iter);
			threadSchedule();
			
		}
			
		(*iter)->vmTick--; 
	}
	

}


TVMMainEntry VMLoadModule(const char *module);
TVMStatus VMFilePrint(int filedescriptor, const char *format, ...);

void idleThread(void*){
	//cout << "idlethread" << endl;
	MachineResumeSignals(&sigstate);
	while(true);
	MachineResumeSignals(&sigstate);

}


TVMStatus VMStart(int tickms, int argc, char *argv[]){
	int flag = false;
	MachineInitialize();
	// request time
	// int time = 0;
	TVMMainEntry mainEntry =  VMLoadModule(argv[0]);
	if(mainEntry == NULL){
		return VM_STATUS_FAILURE;
	}

		


	TMachineAlarmCallback callback = callbackAlarm;
	MachineRequestAlarm(tickms*1000,callback,&flag);
        // setup information for the main and idle thread
       
    //main thread
    TCB* tstartBlk = new TCB;
	tstartBlk->id = threadCount++;
	tstartBlk->idref = 0;
	tstartBlk->priority = VM_THREAD_PRIORITY_NORMAL;
	tstartBlk->state = VM_THREAD_STATE_RUNNING;
	tstartBlk->mmSize = 0;
	tstartBlk->vmTick = 0;
	// SMachineContextRef startContext;
	// MachineContextSwitch(NULL,startContext);

    // SMachineContextRef startContext = new SMachineContext;
    // MachineContextSwitch(NULL,startContext);
    // startB.context = *startContext;

    
    TCB *idleB = new TCB;
    idleB->entryCB = idleThread;
    idleB->priority = VM_THREAD_PRIORITY_LOW;
    idleB->state = VM_THREAD_STATE_READY;
    idleB->mmSize = 0x10000;
    idleB->id = threadCount++;
    SMachineContextRef idleContext = new SMachineContext;
    void* stackaddr = (void*)malloc(idleB->mmSize);
    MachineContextCreate( idleContext, idleB->entryCB , idleB, stackaddr, idleB->mmSize);
    idleB ->context = *idleContext;

    threadList.push_back(tstartBlk);
    threadList.push_back(idleB);
    curThreadID = tstartBlk->id;
    curThread = tstartBlk;
    readyThreads.push(idleB);
    readyThreads.push(tstartBlk);



    mainEntry(argc,argv);
    MachineTerminate();
    return VM_STATUS_SUCCESS;
}

TVMStatus VMTickMS(int *tickmsref){
	if(tickmsref == NULL){
		return VM_STATUS_ERROR_INVALID_PARAMETER;
	}
	*tickmsref = (curThread->vmTick);
	return VM_STATUS_SUCCESS; 
}

TVMStatus VMTickCount(TVMTickRef tickref){
	if(tickref == NULL){
		return VM_STATUS_ERROR_INVALID_PARAMETER;
	}

	return VM_STATUS_SUCCESS; 
}



TVMStatus VMThreadCreate(TVMThreadEntry entry, void *param, TVMMemorySize memsize, TVMThreadPriority prio, TVMThreadIDRef tid){
 MachineSuspendSignals(&sigstate);

	if(entry == NULL || tid == NULL ){
		return VM_STATUS_FAILURE;
	}

	TCB *threadB = new TCB;
	threadB->mmSize = memsize;
	threadB->priority = prio;
	*tid = threadCount;
	threadB->id = threadCount++;

	threadB->entryCB = entry;
	threadB->param = param;
	threadB->state = VM_THREAD_STATE_DEAD;
	threadList.push_back(threadB);

	MachineResumeSignals(&sigstate);

	
return VM_STATUS_SUCCESS; 
}

TVMStatus VMThreadDelete(TVMThreadID thread){
	 MachineSuspendSignals(&sigstate);

	bool found = false;
	vector<TCB*>::iterator iter;
	for(iter = threadList.begin();iter != threadList.end();iter++){
        if((*iter)->id == thread){
        	found = true;
			// delete thread
			threadList.erase(iter);
      }
   }

   if(!found){
   	return VM_STATUS_ERROR_INVALID_ID;
   }
   MachineResumeSignals(&sigstate);

   return VM_STATUS_SUCCESS; 
}

void threadWrapper(void* thread ){
  cout << "Wrapper: " << ((TCB*)thread)->id << endl;
  cout << "HELLO" << endl;
  MachineEnableSignals();
	((TCB*)thread)->entryCB(((TCB*)thread)->param);

	VMThreadTerminate( ((TCB*)thread)->id);
}

TVMStatus VMThreadActivate(TVMThreadID thread){
	// cout << "Activate: " << thread << endl;
	MachineSuspendSignals(&sigstate);

	bool found = false;
	// find  thread with matching id given
	for  (unsigned int i = 0;i < threadList.size();i++){
        if(threadList[i]->id == thread){

        	found = true;
        	if ( threadList[i]->state != VM_THREAD_STATE_DEAD){
        		return VM_STATUS_ERROR_INVALID_STATE;
        	}
        	// activate thread
        	SMachineContextRef  mtContext  = new SMachineContext;
        	void* stackaddr = (void*)malloc(threadList[i]->mmSize);
        	MachineContextCreate( mtContext, threadWrapper , threadList[i], stackaddr, threadList[i]->mmSize);
        	threadList[i]->context = *mtContext;
        	threadList[i]->state = VM_THREAD_STATE_READY;
        	//curThreadID = threadList[i]->id;
    
        	readyThreads.push(threadList[i]);

		}
		
    }
    threadSchedule();

    if(!found){
    	return VM_STATUS_ERROR_INVALID_ID;
    }
    MachineResumeSignals(&sigstate);

    return VM_STATUS_SUCCESS;
}

TVMStatus VMThreadTerminate(TVMThreadID thread){
	// cout << "terminate: " << thread << endl;
	MachineSuspendSignals(&sigstate);

	bool found = false;

	// find  thread with matching id given
	for(unsigned int i = 0;i < threadList.size();i++){
        if(threadList[i]->id == thread){
        	// thread enters the dead state
        	found = true;
        	if(threadList[i]->state == VM_THREAD_STATE_DEAD ){
        		return VM_STATUS_ERROR_INVALID_STATE;
        	}
        	threadList[i]->state = VM_THREAD_STATE_DEAD;
        	// release mutex
        	// schedule other thread
        }
    }
    threadSchedule();

    if(!found){
    	return VM_STATUS_ERROR_INVALID_ID;
    }
    MachineResumeSignals(&sigstate);

    return VM_STATUS_SUCCESS;
}

TVMStatus VMThreadID(TVMThreadIDRef threadref){
	if(threadref == NULL){
		return VM_STATUS_ERROR_INVALID_PARAMETER;
	}

	*threadref = curThreadID;
	return VM_STATUS_SUCCESS;
}

TVMStatus VMThreadState(TVMThreadID thread, TVMThreadStateRef stateref){
	if(stateref == NULL){
		return VM_STATUS_ERROR_INVALID_PARAMETER;
	}
	bool found = false;
	// find  thread with matching id given
	for(unsigned int i = 0;i < threadList.size();i++){
        if(threadList[i]->id == thread){
        	*stateref = threadList[i]->state;
        	found = true;
		}
    }

    if(!found){
    	return VM_STATUS_ERROR_INVALID_ID;
    }
    return VM_STATUS_SUCCESS;	
}




TVMStatus VMThreadSleep(TVMTick tick){
// cout << "sleep thread" << endl;
	TCB* thread;
	if (tick == VM_TIMEOUT_INFINITE){
		return VM_STATUS_ERROR_INVALID_PARAMETER;
	}
	if (tick == VM_TIMEOUT_IMMEDIATE){
		//current process yields the remainder of its processing quantum
		// to the next ready process of equla priority
	}
	else{
	// put current thread to sleep
    vector<TCB*>::iterator iter;
    TCB* test = readyThreads.top();
	  //for(iter = readyThreads.begin();iter != readyThreads.end();iter++){
    //    if((*iter)->id == curThreadID){
	//		    // delete thread
	//		    readyThreads.erase(iter);
   //     }
    //}

		TIMER = tick;
		thread = getTCB(curThreadID);
		thread->vmTick = tick;
		thread->state = VM_THREAD_STATE_WAITING;
    sleepThreads.push_back(thread);
	  threadSchedule();

		//cout << "thread id: " << thread->id << endl;
	

		// while(TIMER != 0){
		// }
		
		
	}

	return VM_STATUS_SUCCESS;
}




volatile bool writeDone = false;
void fileWCallback(void* a,int result){

	writeDone = true;
}

TVMStatus VMFileWrite(int filedescriptor, void *data, int *length){
    // write(filedescriptor,data,*length);
    TMachineFileCallback myfilecallback = fileWCallback;
   
    MachineFileWrite(filedescriptor, data, *length, myfilecallback, NULL);
    // cout << "write: " << calldata << endl;
    while(!writeDone);

    // }

    
    return VM_STATUS_SUCCESS;
}

volatile bool openDone = false; 
int fd;
void fileOpenCallback(void* calldata, int result){
    cout << "CB:" << result << endl;
    fd = result;
   
    openDone = true;
}
 
TVMStatus VMFileOpen(const char *filename, int flags, int mode, int *filedescriptor){
	TMachineFileCallback fOpenCallback = fileOpenCallback;

	MachineFileOpen(filename, flags,  mode, fOpenCallback, NULL);
	while(!openDone);
	*filedescriptor = fd;
	cout << "fileopen:" <<  fd << endl;


 return VM_STATUS_SUCCESS;
 
}

volatile bool closeDone = false;
void fileCloseCallback(void* calldata,int result){
	closeDone = true;
}
TVMStatus VMFileClose(int filedescriptor){
	TMachineFileCallback fCloseCallback = fileCloseCallback;
	
	MachineFileClose(filedescriptor, fCloseCallback, NULL);
	while(!closeDone);
	return VM_STATUS_SUCCESS;
}  

volatile bool seekdone = false;
int offset;
void fileSeekCallback(void* calldata,int result){
	cout << "seek:" << result << endl;
	offset = result;
	seekdone = true;
}

TVMStatus VMFileSeek(int filedescriptor, int offset, int whence, int *newoffset){
	TMachineFileCallback fSeekCallback = fileSeekCallback;
	
	MachineFileSeek( filedescriptor, offset, whence, fSeekCallback, NULL);
	while(!seekdone);
	*newoffset = offset;


    return VM_STATUS_SUCCESS;
}

volatile bool readDone = false;
int readsize; 
void fileReadCallback(void* calldata,int result){
	
	readsize = result;
	readDone = true;
}


TVMStatus VMFileRead(int filedescriptor, void *data, int *length){
	TMachineFileCallback fReadCallback = fileReadCallback;
	
	MachineFileRead(filedescriptor, data, *length, fReadCallback, NULL);
	while(!readDone);
	*length = readsize;


	return VM_STATUS_SUCCESS;
}

 
 

}
