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
    #define VM_THREAD_PRIORITY_IDLE                 ((TVMThreadPriority)0x00)

  volatile int TIMER;
  static const int HIGH = 3;
  static const int MED = 2;
  static const int LOW = 1;
  TMachineSignalState sigstate;

  volatile int threadCount = 1;
  volatile int mutexCount = 1;
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
      int result;
  };

  struct LessThanByPriority{
    bool operator()(const TCB* lhs, const TCB* rhs) const{
      return lhs->priority < rhs->priority;
    }
  };

  typedef priority_queue<TCB*, vector<TCB*>, LessThanByPriority> pq;

  class Mutex{
    public:
      TVMMutexID id;
      bool islocked;
      TCB* owner;

      pq waiting; 
      void nextOwner(){
        owner = waiting.top();
        waiting.pop();
      }

  };



  vector<TCB*> threadList;
  TCB* currentThread;


  pq readyThreads;
  vector<TCB*> sleepThreads;
  vector<Mutex*> mutexList;

  Mutex* getMutex(TVMMutexID mutexId){
    for(unsigned int i = 0; i < mutexList.size(); i++ ){
      if ( mutexId == mutexList[i]->id )
        return mutexList[i];
    }
    return NULL;
  }

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
    // cout << "schedule " << endl;
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
      //cout << "old thread id: " << oldThread->id << endl;
       //cout << "new thread id: " << currentThread->id << endl;

      // cout << "new id: " << currentThread->id << endl;
      if (oldThread->id != currentThread->id){
        //	cout << "switch" << endl;
        SMachineContextRef oldContext = &oldThread->context;
        SMachineContextRef newContext = &currentThread->context; 
        MachineContextSwitch(oldContext,newContext);
      }
    }
    // cout <<  "finish schedule" << endl;
  }

  void idleThread(void*){
   // cout << "idle" << endl;
    MachineResumeSignals(&sigstate);
    while(true);
    MachineResumeSignals(&sigstate);
  }

  void  callbackAlarm( void* t){
   cout << "alarm callback" << endl;
    for(unsigned int i = 0; i < sleepThreads.size();i++){
      if ( sleepThreads[i]->vmTick == 0){
        //	cout << "timeout:" << endl;
        sleepThreads[i]->state = VM_THREAD_STATE_READY;
        //cout << "pushing " << (*iter)->id << endl; 
        readyThreads.push(sleepThreads[i]);
        sleepThreads.erase(sleepThreads.begin() + i);
        threadSchedule();
      }	
      sleepThreads[i]->vmTick--;
    }
  }

  TVMStatus VMThreadSleep(TVMTick tick){
     //cout << "sleep thread " << endl;
    if (tick == VM_TIMEOUT_INFINITE){
      return VM_STATUS_ERROR_INVALID_PARAMETER;
    }
    if (tick == VM_TIMEOUT_IMMEDIATE){
      //current process yields the remainder of its processing quantum
      // to the next ready process of equla priority
    }
    else{
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
    idleB->priority = VM_THREAD_PRIORITY_IDLE;
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
    currentThread = tstartBlk;

    readyThreads.push(idleB);
    mainEntry(argc,argv);

    MachineTerminate();
    return VM_STATUS_SUCCESS;
  }

  TVMStatus VMTickMS(int *tickmsref){
    //cout << "tickms" << endl;
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
   // cout << "entry" << endl;
    MachineSuspendSignals(&sigstate);
    ((TCB*)thread)->entryCB(((TCB*)thread)->param);
    //cout << "ending" << endl;
    VMThreadTerminate( ((TCB*)thread)->id);
    MachineResumeSignals(&sigstate);
  }

  TVMStatus VMThreadActivate(TVMThreadID thread){
    //cout << "Activate: " << thread << endl;
    MachineSuspendSignals(&sigstate);
    TCB* activateTCB = getTCB(thread);
    if (activateTCB != NULL){
      if (activateTCB->state != VM_THREAD_STATE_DEAD )
        return VM_STATUS_ERROR_INVALID_STATE;

      SMachineContextRef  mtContext  = new SMachineContext;
      void* stackaddr = (void*)malloc(activateTCB->mmSize);
      MachineContextCreate( mtContext, threadWrapper , activateTCB, stackaddr, activateTCB->mmSize);
      activateTCB->context = *mtContext;
      activateTCB->state = VM_THREAD_STATE_READY;
      readyThreads.push(activateTCB);

      if ( (activateTCB->priority > currentThread->priority) || (currentThread->state == VM_THREAD_STATE_WAITING)){
        //      	cout << "Activate schedule " << endl;
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
    //	cout << "terminate: " << thread << endl;
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

  TVMStatus VMMutexCreate(TVMMutexIDRef mutexref){
   // cout << "create mutex" << endl;
    if ( mutexref == NULL){
      return VM_STATUS_ERROR_INVALID_PARAMETER;
    }
    Mutex* mutex =  new Mutex;
    mutex->islocked = false;
    mutex->id = mutexCount++;
    *mutexref = mutex->id;
    mutexList.push_back(mutex);
    return VM_STATUS_SUCCESS;
  }

  TVMStatus VMMutexDelete(TVMMutexID mutex){
    bool found = false;
    for (unsigned int i = 0; i < mutexList.size();i++){
      if (mutexList[i]->id == mutex ){
        //check if held by a thread
        mutexList.erase(mutexList.begin() + i );
        found = true;
      }	
    }
    if (!found)
      return VM_STATUS_ERROR_INVALID_ID;



    return VM_STATUS_SUCCESS;
  }

  TVMStatus VMMutexQuery(TVMMutexID mutex, TVMThreadIDRef ownerref){

    if (ownerref == NULL){
      return VM_STATUS_ERROR_INVALID_PARAMETER;
    }
    // mutex 
    Mutex* queryMutex = getMutex(mutex);
    if (queryMutex == NULL){
      return VM_STATUS_ERROR_INVALID_ID;
    }
    if ( queryMutex->islocked ){
      return VM_THREAD_ID_INVALID;
    }
    else{

      *ownerref = queryMutex->owner->id;

    } 

    return VM_STATUS_SUCCESS;
  }

  TVMStatus VMMutexAcquire(TVMMutexID mutex, TVMTick timeout){
   // cout << "acquire" << endl;
    if (timeout == 0){
      return VM_STATUS_FAILURE;
    }
    Mutex* mutexAcquire = getMutex(mutex);

    if (mutexAcquire == NULL ){
      return VM_STATUS_ERROR_INVALID_ID;
    }
    if (timeout == VM_TIMEOUT_IMMEDIATE){
      if (!mutexAcquire->islocked){
        mutexAcquire->islocked = true;
        mutexAcquire->owner = currentThread;
        return VM_STATUS_SUCCESS;
      }
      else{
        return VM_STATUS_FAILURE;
      }

    }
    else if (timeout == VM_TIMEOUT_INFINITE){
      // block
      currentThread->state = VM_THREAD_STATE_WAITING;
      sleepThreads.push_back(currentThread);
      mutexAcquire->waiting.push(currentThread);
    }
    return VM_STATUS_SUCCESS;
  }

  TVMStatus VMMutexRelease(TVMMutexID mutex){
    Mutex* mutexRelease = getMutex(mutex);
    if (mutexRelease == NULL ){
      return VM_STATUS_ERROR_INVALID_ID;
    }
    //cout << "mutex release" << mutexRelease->owner->id << endl;
    // if ( (mutexRelease->owner)->id != currentThread->id ){
    //   return VM_STATUS_ERROR_INVALID_STATE;
    // }
    // mutexRelease->nextOwner();

    return VM_STATUS_SUCCESS;	
  }

  volatile bool writeDone = false;
  void fileWCallback(void* thread,int result){
     MachineSuspendSignals(&sigstate);
       cout << "ready id: " <<  ((TCB*)thread)->id << endl;
    readyThreads.push((TCB*)thread);
    writeDone = true;
    if ( ( (TCB*)thread)->priority > currentThread->priority )
      threadSchedule();
    MachineResumeSignals(&sigstate);

  }

  TVMStatus VMFileWrite(int filedescriptor, void *data, int *length){
     MachineSuspendSignals(&sigstate);
    //cout << "writing" << endl; 
    TMachineFileCallback myfilecallback = fileWCallback; 
    MachineFileWrite(filedescriptor, data, *length, myfilecallback, currentThread);
    // cout << "write: " << calldata << endl;
       //while(!writeDone);
       currentThread->state = VM_THREAD_STATE_WAITING;
       sleepThreads.push_back(currentThread);
       threadSchedule();

    // }
      MachineResumeSignals(&sigstate);


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
