# Operating Systems
University of California, Davis:ECS 150 - Operating Systems

For additional specifications, view the PDFs in each hw directory.

HW1: Design a simple shell that uses POSIX commands, including execution of applications. Operates in non-canonical mode. Special emphasis on forks and piping of multiple commands.

HW2: Provided a machine-layer abstraction, on top of which we build a thread scheduling virtual machine. The virtual machine will load the “applications” from shared objects. The virtual machine will need to support multiple user space preemptive threads. The virtual machine mutex API provides synchronization mechanism and file access is provided through the file API.

HW3: Continued development of the virtual machine. You will be continuing the development of your virtual machine. Adding the ability to create, allocate, deallocate, query, and delete memory pools. The memory pools allow for dynamic memory allocation from specified pools of memory, there is a main system memory pool (VM_MEMORY_POOL_ID_SYSTEM) that is dynamically allocated by the virtual machine at the beginning of execution. The stack space for each of the threads must be allocated from this memory pool. Space is allocated from the memory pools using a first fit algorithm.

HW4: Adding the ability to mount and manipulate a FAT file system image based on the Microsoft white paper on FAT.
