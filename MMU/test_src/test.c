#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
/*
 * Function: main()
 *
 * Description:
 *   Entry point for this program. Enter the appropriate number to 
 *   to perform the test mentioned in the list.
 *
 * Inputs:
 *   argc - The number of argument with which this program was executed.
 *   argv - Array of pointers to strings containing the command-line arguments. 
 *
 * Return value:
 *   0 - This program terminated normally.
 */

void test1();
void test2();
void test3();
void test4();
void test5();
char* get_timestamp ();
 
int
main () {
	int t;
        /* options to perform the tests */
	while(1){
		printf("1: test1(): Test sys_readMMU.\n");
 		printf("2: test2(): Test sys_writeMMU.\n");
		printf("3: test3(): Physical Address for  Virtual Address 0.\n");
		printf("4: test4(): Print physical address for virtual address.\n");
		printf("5: test5(): Map physical address to a new virtual address.\n");
		printf("0: EXIT \n ");
		printf("Input Command: ");
		scanf("%d", &t);
		switch(t){
			case 1: test1(); break;
			case 2: test2(); break;
			case 3: test3(); break;
			case 4: test4(); break;
			case 5: test5(); break;
			case 0:
			default: exit(0);
		}
	}

}

/* function name- test1() 
 * reads from the page table entry 
 * for a virtual address.
 */

void test1() {
	int n;
	unsigned long pte;

	/* read physical address for Virtual address 40 */
	unsigned long a = 40;
	/* get physical addr for respective vaddr from syscall */
        // 	syscall(181);
	//syscall(182);
        n = syscall(181,&a,&pte);
	if(n == -1){
		 printf("readMMU: FAILED!\n");
		 exit(1);
	}else {
		printf("readMMU: VADDR- %lx PTE- %lx\n",&a, pte);
	}
}

/* function name- test2() 
 * reads the page table entry 
 * and write a new physical address 
 * to the virtual address.
 */

void test2() {

	int n;
	unsigned long pte1,pte2;
	unsigned long a = 40 ;
	unsigned long b = 50 ;

	/* read physcial address of vaddr1 */
	n = syscall(181,&a,&pte1);
	if(n == -1){
		 printf("readMMU: FAILED!\n");
	}else {
		printf(" readMMU: VADDR- %lx PTE- %lx\n",&a,pte1);
	}

	/* read physical address of vaddr2 */
	n = syscall(181,&b,&pte2);
	if(n == -1){
		 printf("readMMU: FAILED!\n");
	}else {
		printf("readMMU: VADDR- %lx PTE- %lx\n",&b,pte2);
	}

	/* map vaddr2 to pte1 */
	n = syscall(182, &b, pte1);
	if(n == -1){
		 printf("writeMMU: FAILED!\n");
	}else {
		printf("writeMMU: VADDR- %lx mapped to PTE- %lx\n",&b,pte1);
	}

	/* undo the previous mapping */
	n = syscall(182, &b, pte2);
	if(n == -1){
		 printf("writeMMU: FAILED!\n");
	}else {
		printf("writeMMU: VADDR- %lx mapped to PTE- %lx\n",&b,pte2);
	}
}


/* function name- test3() 
 * reads the page table entry for virtual address zero
 */

void test3() {
	int n;
	unsigned long pte;
	/* read physical address for Virtual address 0 */
	n = syscall(181,0,&pte);
	if(n == -1){
		 printf("readMMU: FAILED!\n");
	}else {
		printf("readMMU: VADDR- 0 PTE- %lx\n", pte);
	}
}

/* function name- test4()
 * syscall readMMU is used to get the page table entry 
 * of the virtual address pointed by mmap
 */

char* get_timestamp () 
{
  time_t now = time (NULL); 
  return asctime (localtime (&now)); 
} 

void test4() {

	int n;
	int *fd;
	unsigned long *vaddr;
	unsigned long pte;
	char print[255];
	
	/*open a file */
	fd = open("foo.txt", O_RDWR|O_CREAT|O_APPEND);
	if(fd == NULL)
	{
		perror("test4(): Unable to open the file!\n");
		exit(1);
	}
	
	struct stat mystat;
	
	if(fstat(fd,&mystat) < 0)
	{
		perror("test4(): fstat error!");
		exit(1);
	}

	/* get virtual  address from mmap*/
	vaddr = (unsigned long *)mmap(0,mystat.st_size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);

        if(vaddr == MAP_FAILED)
		perror("test4(): mmap failed! \n");	

	/* get physical addr for respective vaddr from syscall */
	n = syscall(181,vaddr,&pte);

	if(n == -1){
		 printf("readMMU: FAILED!\n");
	}else {
		printf("readMMU: VADDR- %lx PTE- %lx\n",vaddr, pte);
	}

	char a ; 
	a = vaddr[0]	;
	printf("test4(): value in file A: %c \n", a);

	n = syscall(181,vaddr,&pte);

	if(n == -1){
		 printf("readMMU: FAILED!\n");
		 exit(1);
	}else {
		printf("readMMU results after file access: VADDR- %lx PTE- %lx\n",vaddr, pte);
	}

	close(fd);
}

/* function name- test5()
 * syscall readMMU is used to get the page table entry 
 * of the virtual address pointedd by mmap1 and then mapped 
 * to virtual addrress pointed by mmap2
 */

void test5() {

	int n;
	int fd1,fd2;
	unsigned long *vaddr1,*vaddr2;
	unsigned long pte1 , pte2;
	
	fd1 = open("boo.txt",O_RDWR|O_CREAT|O_APPEND);
	fd2 = open("foo.txt",O_RDWR|O_CREAT|O_APPEND);

	/* read virtual address of mmap */
	struct stat mystat;
	
	if(fstat(fd1,&mystat) < 0)
	{
		perror("test5(): fstat error\n");
		exit(1);
	} 
	
	/* get virtual  address from mmap*/
	vaddr1 = (unsigned long )mmap(0,mystat.st_size,PROT_READ|PROT_WRITE,MAP_SHARED,fd1,0);
        if(vaddr1 == MAP_FAILED)
		perror("test5(): mmap failed\n");	

	char a ; 
	a = vaddr1[0]	;
	printf("test5(): value in file A: %c \n", a);
	
	/* read physcial address of vaddr1 */
	n = syscall(181,vaddr1,&pte1);
	if(n == -1){
		 printf("readMMU: FAILED!\n");
	}else {
		printf("readMMU: VADDR- %lx PTE- %lx\n",vaddr1,pte1);
	}

	/* read virtual address of mmap */
	
	if(fstat(fd2,&mystat) < 0)
	{
		perror("test5(): fstat error!!\n");
		exit(1);
	}

	/* get virtual  address from mmap*/
	vaddr2 = (unsigned long )mmap(0,mystat.st_size,PROT_READ|PROT_WRITE,MAP_SHARED,fd2,0);
        if(vaddr2 == MAP_FAILED)
		perror("test5(): mmap failed\n");	
	
	char b ; 
	b = vaddr2[0]	;
	printf("test5(): value in file 	B: %c \n", a);

	/* read physical address of vaddr2 */
	n = syscall(181,vaddr2,&pte2);
	if(n == -1){
		 printf("readMMU: FAILED!\n");
	}else {
		printf("readMMU: VADDR- %lx PTE- %lx\n",vaddr2,pte2);
	}

	/* map vaddr2 to pte1 */
	n = syscall(182, vaddr2, pte1);
	if(n == -1){
		 printf("writeMMU: FAILED!\n");
	}else {
		printf("writeMMU: VADDR- %lx mapped to PTE- %lx\n",vaddr2,pte1);
	}
	
	/*Write to a virtual address */
	char c = 'X' ; 
	vaddr1[0] = c;	;
	printf("test5(): value written to file A: %c \n",c);
	printf("test5(): value in file B: %c\n", vaddr2[0]);

	/* undo the previous mapping */
	n = syscall(182, vaddr2, pte2);
	if(n == -1){
		 printf("writeMMU: FAILED!\n");
	}else {
		printf("witeMMU: VADDR- %lx mapped to PTE- %lx\n",vaddr2,pte2);
	}

	close(fd1);
	close(fd2);
}
