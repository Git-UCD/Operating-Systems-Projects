#include <iostream> 
#include <sys/types.h> 
#include <sys/wait.h> 
#include <unistd.h> 
 #include <cstdlib>
using std::cout; 
using std::endl; 
using std::cerr; 
 
int main(int argc, char * argv[]) 
{ 
   int pid = -1; 
 
   // Create a child process 
   pid = fork(); 
 
   if(pid == 0) 
   { 
     cout << endl << "I am the child. My pid is " << getpid() << "." << endl; 
      return 0; 
   } 
   else if(pid > 0) 
   { 
      cout  << endl << "I am the parent. My pid is " << getpid() << "." << endl; 
   } 
   else 
   { 
      cerr << "Fork error. Quitting Program." << endl; 
 
      // Bail out of the program. 
      exit(EXIT_FAILURE); 
   } 
 
   // The parent will wait for a child to die before proceeding.  The child 
   // will never get here, because the child quits in the if statement above. 
   wait(0); 
   cout  << endl << "Child just died." << endl; 
 
   // Now, the child is dead for sure, and we are back to one process. 
 
   cout << endl << endl; 
   cout << endl << "Starting phase 2." << endl; 
   cout << endl << endl; 
 
   // Fork a process, and have the child execute an ls. 
   pid = fork(); 
 
   if(pid == 0) 
   { 
      cout << endl << "I am the child. My pid is " << getpid() << "." << endl; 
      cout << endl << "The child will now exec an ls command." << endl; 
 
      // Print a line of stars above the ls. 
      cout << "****************************************"  
         << "****************************************" << endl; 
      char *argm[] = {"ls", "-la", 0}; 
      
      execvp(argm[0], argm);
 
      cout << "This statement should not be executed if execvp is successful." << endl; 
      cerr << "The exec command issued by the child failed!!!  Exiting."  
         << endl; 
      exit(EXIT_FAILURE); 
   } 
   else if(pid > 0) 
   { 
      wait(0); 
 
      // We know for sure that the child is dead, so print a line of stars 
      // after the ls. 
      cout << endl << "Parent: " << "****************************************"  
         << "****************************************" << endl; 
   } 
   else 
   { 
      cerr << "Fork error. Quitting Program." << endl; 
 
      // Bail out of the program. 
      exit(EXIT_FAILURE); 
   } 
 
   return 0; 
} 
