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
TVMMainEntry VMLoadModule(const char *module);
TVMStatus VMFilePrint(int filedescriptor, const char *format, ...);



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
TCB* currentThread;

struct LessThanByPriority{
  bool operator()(const TCB* lhs, const TCB* rhs) const{
    return lhs->priority < rhs->priority;
  }
};
typedef priority_queue<TCB*, vector<TCB*>, LessThanByPriority> pq;


pq readyThreads;
vector<TCB*> sleepThreads;

TCB* getTCB(TVMThreadID id){
	for (unsigned int i = 0;i < threadList.size();i++){
		if (id == threadList[i]->id){
			return threadList[i];
		}
	}
	return NULL;
}


void threadSchedule(){
  TCB* oldThread = currentThread;
	//TCB* nextThread;
  cout << "schedule " << endl;
  //cout << "current id: " << currentThread->id <<  endl;
  //cout << "readyThread: " << readyThreads.top()->id << endl;
 // cout << "size: " << readyThreads.size() << endl;
 if (readyThreads.empty() ){
   readyThreads.push(getTCB(2));
}
  if(!readyThreads.empty( )){
 	
	currentThread = readyThreads.top();
    currentThread->state = VM_THREAD_STATE_RUNNING;
    readyThreads.pop();
    cout << "old thread id: " << oldThread->id << endl;
    cout << "new thread id: " << currentThread->id << endl;

   // cout << "new id: " << currentThread->id << endl;
	if (oldThread->id != currentThread->id){
		cout << "switch" << endl;
		SMachineContextRef oldContext = &oldThread->context;
		SMachineContextRef newContext = &currentThread->context; 
		MachineContextSwitch(oldContext,newContext);
	}
	
	

	
}



  cout <<  "finish schedule" << endl;

}



void idleThread(void*){
	 cout << "idlethread" << endl;
	MachineResumeSignals(&sigstate);
	while(true);
	MachineResumeSignals(&sigstate);
}

void  callbackAlarm( void* t){
	 cout << "callback" << endl;
	//TIMER--
	for(vector<TCB*>::iterator iter = sleepThreads.begin(); iter != sleepThreads.end();iter++){
        cout << "thread id: " <<  (*iter)->id << endl;
	cout << "thread ticks: " << (*iter)->vmTick << endl;
		if ( (*iter)->vmTick == 0){
			cout << "timeout:" << endl;
			(*iter)->state = VM_THREAD_STATE_READY;
                        cout << "pushing " << (*iter)->id << endl; 
			readyThreads.push(*iter);
			sleepThreads.erase(iter);
			threadSchedule();
				
		}	
		(*iter)->vmTick--;

	}
}

TVMStatus VMThreadSleep(TVMTick tick){
 cout << "sleep thread " << endl;
	if (tick == VM_TIMEOUT_INFINITE){
		return VM_STATUS_ERROR_INVALID_PARAMETER;
	}
	if (tick == VM_TIMEOUT_IMMEDIATE){
		//current process yields the remainder of its processing quantum
		// to the next ready process of equla priority
	}
	else{
	// put current thread to sleep
		// TIMER = tick;
		//thread = getTCB(curThreadID);
	cout << "sleep thread: " <<  currentThread->id << endl;
        cout << "sleep ticks: " << tick << endl;	
		currentThread->vmTick = tick;
		currentThread->state = VM_THREAD_STATE_WAITING;
		sleepThreads.push_back(currentThread);
		threadSchedule();
		 //cout << "sleep thread id: " << currentThread->id << endl;		
	}
	//threadSchedule();

	return VM_STATUS_SUCCESS;
}



TVMStatus VMStart(int tickms, int argc, char *argv[]){
	MachineInitialize();
	// request time
	TVMMainEntry mainEntry =  VMLoadModule(argv[0]);
	if(mainEntry == NULL){
		return VM_STATUS_FAILURE;
	}
	TMachineAlarmCallback callback = callbackAlarm;
	MachineRequestAlarm(tickms*1000,callback,NULL);
    // setup information for the main and idle thread
    //main thread
        TCB* tstartBlk = new TCB;
	tstartBlk->id = threadCount++;
	tstartBlk->idref = 0;
	tstartBlk->priority = VM_THREAD_PRIORITY_NORMAL;
	tstartBlk->state = VM_THREAD_STATE_RUNNING;
	tstartBlk->mmSize = 0;
	tstartBlk->vmTick = 0;
    // idle thread
    TCB *idleB = new TCB;
    idleB->entryCB = idleThread;
    idleB->priority = VM_THREAD_PRIORITY_LOW;
    idleB->state = VM_THREAD_STATE_READY;
    idleB->mmSize = 0x10000;
    idleB->vmTick = 0;
    idleB->id = threadCount++;
    SMachineContextRef idleContext = new SMachineContext;
    void* stackaddr = (void*)malloc(idleB->mmSize);
    MachineContextCreate( idleContext, idleB->entryCB , idleB, stackaddr, idleB->mmSize);
    idleB ->context = *idleContext;

    threadList.push_back(tstartBlk);
    threadList.push_back(idleB);
    curThreadID = tstartBlk->id;
    currentThread = tstartBlk;

    readyThreads.push(idleB);

  

    mainEntry(argc,argv);

    MachineTerminate();
    return VM_STATUS_SUCCESS;
}

TVMStatus VMTickMS(int *tickmsref){
cout << "tickms" << endl;
	if(tickmsref == NULL){
		return VM_STATUS_ERROR_INVALID_PARAMETER;
	}
	*tickmsref = (currentThread->vmTick);
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
       cout << "entry" << endl;
	MachineSuspendSignals(&sigstate);

	((TCB*)thread)->entryCB(((TCB*)thread)->param);
	//cout << "ending" << endl;
	VMThreadTerminate( ((TCB*)thread)->id);
        MachineResumeSignals(&sigstate);
}

TVMStatus VMThreadActivate(TVMThreadID thread){
	cout << "Activate: " << thread << endl;
	MachineSuspendSignals(&sigstate);
	TCB* activateTCB = getTCB(thread);
	if (activateTCB != NULL){
		if (activateTCB->state != VM_THREAD_STATE_DEAD ){
			return VM_STATUS_ERROR_INVALID_STATE;
		}
		SMachineContextRef  mtContext  = new SMachineContext;
		void* stackaddr = (void*)malloc(activateTCB->mmSize);
        MachineContextCreate( mtContext, threadWrapper , activateTCB, stackaddr, activateTCB->mmSize);
        activateTCB->context = *mtContext;
        activateTCB->state = VM_THREAD_STATE_READY;
        readyThreads.push(activateTCB);
        cout << "current state: " << currentThread->state << endl;
         if ( (activateTCB->priority > currentThread->priority) || (currentThread->state == VM_THREAD_STATE_WAITING)){
        	cout << "Activate schedule " << endl;
         threadSchedule();
           }
	}
	else{
	  return VM_STATUS_ERROR_INVALID_ID;
	}

    MachineResumeSignals(&sigstate);
    return VM_STATUS_SUCCESS;
}

TVMStatus VMThreadTerminate(TVMThreadID thread){
	cout << "terminate: " << thread << endl;
	MachineSuspendSignals(&sigstate);
	TCB* terminateTCB = getTCB(thread);
	if (terminateTCB != NULL){
		if(terminateTCB == VM_THREAD_STATE_DEAD){
			return VM_STATUS_ERROR_INVALID_STATE;
		}
		terminateTCB->state = VM_THREAD_STATE_DEAD;
	}
	else{
		return VM_STATUS_ERROR_INVALID_ID;
	}
        threadSchedule();
    MachineResumeSignals(&sigstate);
    return VM_STATUS_SUCCESS;
}

TVMStatus VMThreadID(TVMThreadIDRef threadref){
    	if(threadref == NULL){
		return VM_STATUS_ERROR_INVALID_PARAMETER;
	}

	*threadref = currentThread->id;
	return VM_STATUS_SUCCESS;
}


TVMStatus VMThreadState(TVMThreadID thread, TVMThreadStateRef stateref){

	if(stateref == NULL){
		return VM_STATUS_ERROR_INVALID_PARAMETER;
	}
	TCB* stateTCB = getTCB(thread);
	if(stateTCB != NULL){
		*stateref = stateTCB->state;
	}
	else{
		return VM_STATUS_ERROR_INVALID_ID;
	}
    return VM_STATUS_SUCCESS;	
}


volatile bool writeDone = false;
void fileWCallback(void* thread,int result){
//        cout << "ready id: " <<  ((TCB*)thread)->id << endl;
//        readyThreads.push((TCB*)thread);
	writeDone = true;
//       if ( ( (TCB*)thread)->priority > currentThread->priority )
//        threadSchedule();
}

TVMStatus VMFileWrite(int filedescriptor, void *data, int *length){
//cout << "writing" << endl; 
    TMachineFileCallback myfilecallback = fileWCallback;
   
    MachineFileWrite(filedescriptor, data, *length, myfilecallback, currentThread);
    // cout << "write: " << calldata << endl;
    while(!writeDone);

  // sleepThreads.push_back(currentThread);
  // threadSchedule();

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
