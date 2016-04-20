#include "VirtualMachine.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#define SMALL_BUFFER_SIZE       256

void *VMLibraryHandle = NULL;

TVMMainEntry VMLoadModule(const char *module){
    
    VMLibraryHandle = dlopen(module, RTLD_NOW);
    if(NULL == VMLibraryHandle){
        fprintf(stderr,"Error dlopen failed %s\n",dlerror());
        return NULL;
    }
    
    return (TVMMainEntry)dlsym(VMLibraryHandle, "VMMain");
}

void VMUnloadModule(void){
    if(NULL != VMLibraryHandle){
        dlclose(VMLibraryHandle);
    }
    VMLibraryHandle = NULL;
}

TVMStatus VMFilePrint(int filedescriptor, const char *format, ...){
    va_list ParamList;
    char *OutputBuffer;
    char SmallBuffer[SMALL_BUFFER_SIZE];
    int SizeRequired;
    TVMStatus ReturnValue;

    va_start(ParamList, format);
    OutputBuffer = SmallBuffer;

    SizeRequired = vsnprintf(OutputBuffer, SMALL_BUFFER_SIZE, format, ParamList);
    if(SizeRequired < SMALL_BUFFER_SIZE){
        ReturnValue = VMFileWrite(filedescriptor, OutputBuffer, &SizeRequired);
        return ReturnValue;
    }
    OutputBuffer = (char *)malloc(sizeof(char) *(SizeRequired + 1));
    SizeRequired = vsnprintf(OutputBuffer, SizeRequired, format, ParamList);
    ReturnValue = VMFileWrite(filedescriptor, OutputBuffer, &SizeRequired);
    free(OutputBuffer);
    return ReturnValue;
}

