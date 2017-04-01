
************* Files modified or added to the kernel source code **************

1. Modified linux-3.18.42/arch/x86/syscalls/syscall_64.tbl
2. Added floder 'csc456' to linux-3.18.42
2. Added “mmu.h”,"mmu.c" to linux-3.18.42/csc456
4. Modified linux-3.18.42/kernel/Makefile

*********************************** project **********************************

Chinmai - Syscalls implemented:
	  181- sys_readMMU() 
	  182- sys_writeMMU() 
	  Test_programs:
		1 : Performs the test for sys_readMMU.
		2 : Performs test for sys_writeMMU.
		3 : Reads Physical address for virtual address 0.
		4 : Reads Physical address for virtual address 
		    of file with memory allocated by mmap. 
		5 : Maps Physical-address1 to virtual-address 2
		    modifies file1 to observe changes in file2. 

******************************* How to run test file ****************************

test_src : contains test.c and makefile for test 
compile  : make test

Step1 : run - ./test
	This will provide a list of 5 tests.
Step2 : Input Command:
	Enter the integer corresponding to the test to run. 
Step3 : Perform tests with help of options provided by the list.


