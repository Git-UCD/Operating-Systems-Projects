#include "VirtualMachine.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]){
    int TickTimeMS = 100;
    int Offset = 1;
    
    while(Offset < argc){
        if(0 == strcmp(argv[Offset], "-t")){
            // Tick time in ms
            Offset++;
            if(Offset >= argc){
                break;   
            }
            if(1 != sscanf(argv[Offset],"%d",&TickTimeMS)){
                fprintf(stderr,"Invalid parameter for -t of \"%s\".\n",argv[Offset]);
                return 1;
            }
            if(0 >= TickTimeMS){
                fprintf(stderr,"Invalid parameter for -t must be positive!\n"); 
                return 1;
            }
        }
        else{
            break;
        }
        Offset++;
    }
    
    if(Offset >= argc){
        fprintf(stderr,"Syntax Error: vm [options] module [moduleoptions]\n");    
        return 1;
    }
    
    
    if(VM_STATUS_SUCCESS != VMStart(TickTimeMS, argc - Offset, argv + Offset)){
        fprintf(stderr,"Virtual Machine failed to start.\n");    
        return 1;
    }
    
    
    return 0;
}




