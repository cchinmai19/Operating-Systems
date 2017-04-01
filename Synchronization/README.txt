
************* Files modified or added to the kernel source code **************

1. Modified linux-3.18.42/arch/x86/syscalls/syscall_64.tbl
2. Added “event.h” to linux-3.18.42/include/linux
3. Added “event.c” to llinux-3.18.42/kernel
4. Modified linux-3.18.42/kernel/Makefile
5. Modified linux-3.18.42/init/main.c


***************** project distribution between partners ***********************


Chinmai - Syscalls implemented:
	  181- doeventopen() 182-doeventclose() 183-doeventwait() 184-doeventsig()
	  Test_programs:
	  Event Access Controls test:
		1 : Creates new events
		2 : Destroy the event
		3 : Blocks process
		4 : Unblocks all waiting processes


Tiecheng Su - Syscalls implemented:
	  185- doeventinfo 205-doeventchown() 211-doeventchmod() 214-doeventstat()
	  Test_programs:
	  Event Access Controls test:
		5 : event info
		6 : Change own
		7 : Change mode
		8 : event state
	  Boundary condition test programs: 
		9 : doeventsig while no tasks waiting
		10 : doeventsig while multiple tasks waiting
		11 : multiple events open at once and close
		     (POSIX pthreads were used to create multiple events at once)
		12 : doeventclose while processes waiting
		13 : Change process own
		0  : exit

******************************* How to run test file ****************************

test_src : contains test.c and makefile for test 
compile  : make test

Step1 : run - ./test
	This will provide a list of 13 tests.
Step2 : Input Command:
	Enter the integer corresponding to the test to run.
Step3 : Select option 13 to change the effective UID and effective GID
	of the process. 
Step4 : Perform access controls tests with help of options provided by the list.


