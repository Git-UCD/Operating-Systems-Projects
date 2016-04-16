#include "VirtualMachine.h"
#include "Machine.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> // delete later, used for write
#include <iostream>
using namespace std;


extern "C"{
  TVMMainEntry VMLoadModule(const char *module);
// put all functions/prototypes here
  TVMStatus VMStart(int tickms, int argc, char *argv[]){
    MachineInitialize();
    TVMMainEntry mainEntry = VMLoadModule(argv[0]);
    mainEntry(argc,argv);   
    return(VM_STATUS_SUCCESS);
  }

  TVMStatus VMFileWrite(int filedescriptor, void *data, int *length){
    write(filedescriptor, data, *length);
    //MachineFileWrite(file
    return(VM_STATUS_SUCCESS);
  }
}
