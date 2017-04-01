/* 
 * tsh - A tiny shell program with job control
 * AUTHOR - CHINMAI 
 * URID - 29471805
 * <Put your name and login ID here>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* Misc manifest constants */
#define EXITCONDITION 42  /* exit from function to main*/
#define MAXLINE    1024   /* max line size */
#define MAXARGS     128   /* max args on a command line */
#define MAXJOBS      16   /* max jobs at any point in time */
#define MAXJID    1<<16   /* max job ID */
/* Job states */
#define UNDEF 0 /* undefined */
#define FG 1    /* running in foreground */
#define BG 2    /* running in background */
#define ST 3    /* stopped */
#define D  4    /* Suspended */
/* 
 * Jobs states: FG (foreground), BG (background), ST (stopped)
 * Job state transitions and enabling actions:
 *     FG -> ST  : ctrl-z
 *     ST -> FG  : fg command
 *     ST -> BG  : bg command
 *     BG -> FG  : fg command
 * At most 1 job can be in the FG state.
 */

/* Global variables */
extern char **environ;      /* defined in libc */
char prompt[] = "tsh> ";    /* command line prompt (DO NOT CHANGE) */
int verbose = 0;            /* if true, print additional output */
int nextjid = 1;            /* next job ID to allocate */
char sbuf[MAXLINE];         /* for composing sprintf messages */

struct job_t {              /* The job struct */
    pid_t pid;              /* job PID */
    int jid;                /* job ID [1, 2, ...] */
    int state;              /* UNDEF, BG, FG, or ST */
    char cmdline[MAXLINE];  /* command line */
};
struct job_t jobs[MAXJOBS]; /* The job list */
/* End global variables */


/* Function prototypes */

/* Here are the functions that you will implement */
void eval(char *cmdline);
int builtin_cmd(char **argv);
void do_bgfg(char **argv);
void waitfg(pid_t pid);

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);

/* Here are helper routines that we've provided for you */
int parseline(const char *cmdline, char **argv); 
void sigquit_handler(int sig);

void clearjob(struct job_t *job);
void initjobs(struct job_t *jobs);
int maxjid(struct job_t *jobs); 
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline);
int deletejob(struct job_t *jobs, pid_t pid); 
pid_t fgpid(struct job_t *jobs);
struct job_t *getjobpid(struct job_t *jobs, pid_t pid);
struct job_t *getjobjid(struct job_t *jobs, int jid); 
int pid2jid(pid_t pid); 
void listjobs(struct job_t *jobs);

void usage(void);
void unix_error(char *msg);
void app_error(char *msg);
typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler);

/*support functions*/
void  execute(char **argv,int i );
void  pipe_execute(char **argv, int p, int i);
/*
 * main - The shell's main routine 
 */
int main(int argc, char **argv) 
{
    char c;
    char cmdline[MAXLINE];
    int emit_prompt = 1; /* emit prompt (default) */
  //    cmdline = (char*)malloc(MAXLINE*sizeof(char*));
    int temp_flag = 0;

    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) */
    dup2(1, 2);
	  
    /* Parse the command line */
    while ((c = getopt(argc, argv, "hvp")) != EOF) {
        switch (c) {
        case 'h':             /* print help message */
            usage();
	    break;
        case 'v':             /* emit additional diagnostic info */
            verbose = 1;
	    break;
        case 'p':             /* don't print a prompt */
            emit_prompt = 0;  /* handy for automatic testing */
	    break;
	default:
            usage();
	}
    }

    /* Install the signal handlers */

    /* These are the ones you will need to implement */
    Signal(SIGINT,  sigint_handler);   /* ctrl-c */
    Signal(SIGTSTP, sigtstp_handler);  /* ctrl-z */
    Signal(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */

    /* This one provides a clean way to kill the shell */
    Signal(SIGQUIT, sigquit_handler); 

    /* Initialize the job list */
    initjobs(jobs);
        /* Execute the shell's read/eval loop */        
    while (1) {
	
	temp_flag = 0;
	/* Read command line */
		  //  printf("poor program\n");
	if (emit_prompt) {
	    printf("%s", prompt);
	    fflush(stdout);
	}
		   // printf("poor program\n");
	if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin))
	    app_error("fgets error");       
	if (feof(stdin)) { /* End of file (ctrl-d) */
	    fflush(stdout);
	    exit(0);
	} 
 	/* Evaluate the command line */
	if (strlen(cmdline) == 1 && cmdline[0] == 10)
		temp_flag = 1;

       //printf("%d crap ass useless %d \n",strlen(cmdline),(int)cmdline[0]);
       if(strlen(cmdline) > 0 && cmdline != NULL && temp_flag == 0){
		eval(cmdline);
	}
	fflush(stdout);
	fflush(stdout);
    } 

    exit(0); /* control never reaches here */
}
  
/* 
 * eval - Evaluate the command line that the user has just typed in
 * 
 * If the user has requested a built-in command (quit, jobs, bg or fg)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.  
*/
void eval(char *cmdline) 
{   
    char *cmd[MAXARGS];
    int start_bgfg,p,j = 0,i;
    p = 0 ; 
    for(i = 0; i < MAXARGS; i++){
   	 cmd[i] = (char*)malloc(MAXLINE);
 	 } 
    start_bgfg = parseline(cmdline,cmd); // get flag if the process is in background and foreground

 //  printf("poor program\n");
    while(cmdline[j] != '\0'){
	   if(cmdline[j] >= '|')
		   p = p + 1;
	   j = j+1;
   }
    
//printf("number pipes: %d", p);

  if((strcmp(cmd[0],"cd")== 0) || (strcmp(cmd[0],"exit")== 0) || (strcmp(cmd[0], "jobs") == 0) ){
            builtin_cmd(cmd);
          } 
    else if((strcmp(cmd[0],"fg") == 0) || (strcmp(cmd[0],"bg") == 0)){
	    do_bgfg(cmd); // commmand has fg and bg execute do_bgfg
	}
    else{
	    if(p != 0){
		   // printf("I WILL EXECUTE PIPE FUNCTION\n");
		    pipe_execute(&cmdline,p,start_bgfg); // if there are pipes in the command call pipe_execute
	    }
	    else{
                    //printf("poor program\n");

		    execute(cmd,start_bgfg);

	    }
        }       
    return;
}

/* 
 * parseline - Parse the command line and build the argv array.
 * 
 * Characters enclosed in single quotes are treated as a single
 * argument.  Return true if the user has requested a BG job, false if
 * the user has requested a FG job.  
 */
int parseline(const char *cmdline, char **argv) 
{
    static char array[MAXLINE]; /* holds local copy of command line */
    char *buf = array;          /* ptr that traverses command line */
    char *delim;                /* points to first space delimiter */
    int argc;                   /* number of args */
    int bg;                     /* background job? */

    strcpy(buf, cmdline);
    buf[strlen(buf)-1] = ' ';  /* replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* ignore leading spaces */
	buf++;

    /* Build the argv list */
    argc = 0;
    if (*buf == '\'') {
	buf++;
	delim = strchr(buf, '\'');
    }
    else {
	delim = strchr(buf, ' ');
    }

    while (delim) {
	argv[argc++] = buf;
	*delim = '\0';
	buf = delim + 1;
	while (*buf && (*buf == ' ')) /* ignore spaces */
	       buf++;

	if (*buf == '\'') {
	    buf++;
	    delim = strchr(buf, '\'');
	}
	else {
	    delim = strchr(buf, ' ');
	}
    }
    argv[argc] = NULL;
    
    if (argc == 0)  /* ignore blank line */
	return 1;

    /* should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0) {
	argv[--argc] = NULL;
    }
    return bg;
}

/* 
 * builtin_cmd - If the user has typed a built-in command then execute
 *    it immediately.  
 */
 
int builtin_cmd(char **argv) 
{
     pid_t  pid;
     int    status;

     if((strcmp(argv[0],"cd") == 0)){
           if(chdir(argv[1]) <0){
           }
         }
     else if((strcmp(argv[0],"jobs")) == 0){
          listjobs(jobs);
         }
     else if((strcmp(argv[0],"exit")) == 0){
          exit(1);
         }
     
    return 0;     
}

/*** invokes many child processes and file descriptors to perform commands
 * which require input from the previous process***/
void  pipe_execute(char **argv, int p, int start_bgfg){

	pid_t pid;
	int status,in;
	int fd[2*p];
	int k,i,w,flag ;
        char *cmd[MAXARGS];
        char *cmd1[10];
	char *pch;
	struct job_t *job;

	flag = 0;
	
	for(k = 0; k < MAXARGS; k++){
    		cmd[k] = (char*)malloc(30*sizeof(char));
		cmd[k][0] = '\0';
  	}
        
	for(k = 0; k < 10; k++){
		cmd1[k] = (char*)malloc(40*sizeof(char));
		cmd1[k][0] = '\0';
	}
        
        pch = strtok(*argv,"|"); // spilt the commandline into tokens with | as a delimiter
	k = 0;
	cmd[k++] = (char *)pch;
	while (pch != NULL)
	{      
		pch = strtok(NULL,"|");
                cmd[k] = (char *)pch;
		k = k+1;
        }
	
if(start_bgfg != 1){  // check if it is background task or not
		for(i=0; i < p+1;i++){
       			for(k = 0; k < 10; k++){
				cmd1[k] = '\0';
				}
                	k = parseline(cmd[i],&(*cmd1)); // command and spilt it into words
			if(pipe(&fd[2*i]) < 0){ // creating pipes
				app_error("error creating pipe\n");
				exit(1);
			}
                        
			if((pid = fork()) < 0){
			       app_error("*** ERROR: forking child process failed\n");
			       exit(1);	
			}

			else if(pid == 0){ // if it is a child process then execute
			    
			    if( i == p+1) // perform the below if it is the last chiild
			     {
			      if(dup2(0, fd[(i*2)]) < -1) // copy stdin to read end
			      {
				      app_error("dup input to read end failed\n");
				      exit(1);
			      }
			      if(close(fd[i*2]) < 0)// close read_end
			      {
				      app_error("Read end close failed\n");
				      exit(1);
			      }
			      k = parseline(cmd[i], &(*cmd1));
			      if(execvp(*cmd1,cmd1)<0) // execute 
			      {
				      printf("%s command not found\n",*argv);
				      exit(1);
			      }
			      if(dup2(fd[(i*2)-1],1) < -1) // copy write end to stdout
			      {
				      app_error("dup output to wrte end failed\n");
				      exit(1);
			      }
			      if(close(fd[(i*2)-1]) < 0) //close write end of the file descriptor
			      {
				      app_error("Write end close failed\n");
				      exit(1);
			      }
			    }
			    else
			    {

				if(dup2(0, fd[(i*2)]) < -1)
			        {
				      app_error("dup input to read end failed\n");
				      exit(1);
			        }
				if(close(fd[i*2]) < 0)
			        {
				      app_error("Read end close failed\n");
				      exit(1);
			        }
				k = parseline(cmd[i],&(*cmd1));
				if(execvp(*cmd1,cmd1) < 0){
					printf("%s: command not found\n",*argv);
					exit(1);
				}
				if(dup2(fd[(i*2)-1],1) < -1)
			        {
				      app_error("dup output to wrte end failed\n");
				      exit(1);
			        }
				if(close(fd[i*2]-1) < 0)
			        {
				      app_error("Read end close failed\n");
				      exit(1);
			        }

			    }
			}
			else{
		   	    do{ 
			        if(i ==0) {in = addjob(jobs,pid, FG, *argv); } 
			    	flag = 0;
			    	w = waitpid(pid,&status,WUNTRACED);
                            	if(w == -1){app_error("WaitPid"); exit(1);}
		            	if(WIFEXITED(status)){flag = 1;
                                in = deletejob(jobs,pid); // delete process if child has exited
				}
				else if(WIFSIGNALED(status)){
			        flag = 1; // delete the job if signal 126 is received
			        printf("Unable to execute: Killed by signal %d\n", WTERMSIG(status));
			        in = deletejob(jobs,pid);
			    }
				else if(WIFSTOPPED(status))
				{
				    flag = 1;// suspend the process if ctrl +z is received
			     	    printf("Suspended \n");
				    job = getjobpid(jobs,pid);
		              	    (*job).state = ST;
			        }
		              }while(flag != 1);	
			}		  
  		}
   }
   else
	{  // Running process in the background without waiting
          	for(i=0; i < p+1;i++){
       			for(k = 0; k < 10; k++){
				cmd1[k] = '\0';
				}
                	k = parseline(cmd[i],&(*cmd1));
			if(pipe(&fd[2*i]) < 0){
				app_error("Error creating pipe\n");
				exit(1);
			}
                        
			if((pid = fork()) < 0){
			       app_error("*** ERROR: forking child process failed\n");
			       exit(1);	
			}

			else if(pid == 0){
			      
			    
			    if( i == p+1)
			     {
			      if(dup2(0, fd[(i*2)]) < -1) // copy stdin to read end of the file descriptor
			      {
				      app_error("dup input to read end failed\n");
				      exit(1);
			      }
			      if(close(fd[i*2]) < 0)
			      {
				      app_error("Read end close failed\n");
				      exit(1);
			      }
			      k = parseline(cmd[i], &(*cmd1));
			      if(execvp(*cmd1,cmd1)<0)
			      {
				      printf("%s: command not found\n",*argv);
				      exit(1);
			      }
			      if(dup2(fd[(i*2)-1],1) < -1)
			      {
				      app_error("dup output to wrte end failed\n");
				      exit(1);
			      }
			      if(close(fd[(i*2)-1]) < 0)
			      {
				      app_error("Write end close failed\n");
				      exit(1);
			      }
			    }else
			    {

				if(dup2(0, fd[(i*2)]) < -1)
			        {
				      app_error("dup input to read end failed\n");
				      exit(1);
			        }
				if(close(fd[i*2]) < 0)
			        {
				      app_error("Read end close failed\n");
				      exit(1);

			        }
				k = parseline(cmd[i],&(*cmd1));
				if(execvp(*cmd1,cmd1) < 0){
					printf("%s: command not found\n",*argv);
					exit(1);
				}
				if(dup2(fd[(i*2)-1],1) < -1)
			        {
				      app_error("dup output to wrte end failed\n");
				      exit(1);

			        }
				if(close(fd[i*2]-1) < 0)
			        {
				      app_error("Read end close failed\n");
                                      exit(1);

				      
			        }

			    }
			}
			else{
		            if(i == 0) { in = addjob(jobs,pid, BG, *argv); 
			 }// add the first job to the job q
			}
		}
	}
   return ;
}


/*** executes the process by forking a child and parent waits
 in the forground until the child has completed ***/


void  execute(char **argv, int i)
{
     pid_t  pid;
     int    status,in,w,flag,q;
     struct job_t *job;  

     if(i != 1){ // checks if the process is backgroud or foreground
	     if ((pid = fork()) < 0) {    
                printf("*** ERROR: forking child process failed\n");
                exit(1);
           }
           else if (pid == 0) {   
	            if (execvp(*argv, argv) < 0) {   
                    printf("%s: command not found\n",*argv);
                     exit(1);
                }
	           }
           else {
                 in = addjob(jobs, pid , FG, *argv);                  
		do{        
			    flag = 0;
			    w = waitpid(pid,&status,WUNTRACED);
                            if(w == -1){app_error("WaitPid"); exit(1);}
		            if(WIFEXITED(status)){
			      flag = 1;
			      in = deletejob(jobs,pid);
			     }else if(WIFSIGNALED(status)){
			       flag = 1;
			       printf("Unable to execute: Killed by signal %d\n", WTERMSIG(status));
			       printf("[%d] +Deleted\n",pid);
			       in = deletejob(jobs,pid);
			    }else if(WIFSTOPPED(status)){
				    flag = 1;
			      printf("Suspended \n");
		              job = getjobpid(jobs,pid);
			      (*job).state = ST;
			    }
		}while(flag != 1);	
	
                }
     }
    else {
	   if ((pid = fork()) < 0) {    
                printf("*** ERROR: forking child process failed\n");
               exit(1);
           }
           else if (pid == 0) {
		Signal(SIGINT,sigint_handler); 
                if (execvp(*argv, argv) < 0) { 
	                     printf("%s: command not found\n",*argv);
	                     exit(1);}             
	    }
            else {  printf("[%d]\n",pid);
	            in = addjob(jobs, pid , BG, *argv);
		    	     }
	         }
    return;
}

/* 
 * do_bgfg - Execute the builtin bg and fg commandsi
 */
void do_bgfg(char **argv) 
{ 
  pid_t pid;
  int status,flag,start_bgfg;
  int jid,in,i,w;
  char *cmd[MAXARGS];
  struct job_t *job;

  start_bgfg = 0; // checks if the process is still in the background
  jid = atoi(argv[1]);

  for(i = 0; i < MAXARGS; i++){
    cmd[i] = (char*)malloc(MAXLINE);
  }

  if(getjobjid(jobs,jid) != NULL){
	  start_bgfg = 1;
	  job = getjobjid(jobs,jid);
  }



if(start_bgfg){ // if command is bg job_id
	  if(strcmp(argv[0], "bg") == 0){
		  if((*job).state = ST)
		  {      (*job).state = BG;
			  printf("[%d]\n",(*job).pid);
 			  if(kill((*job).pid,SIGCONT)){ // resume the job by sending a continue signal
			   printf("[%d]  +Running   %s\n",(*job).jid,(*job).cmdline);
			  }
		  }
		}
	  else if (strcmp(argv[0],"fg") == 0){ // if command is fg job id
                 (*job).state = FG;
		 if((*job).state = ST) {
			  if(kill((*job).pid,SIGCONT)){ // send continue signal to resume the job
			    printf("[%d]  +Running   %s\n",(*job).jid,(*job).cmdline);
			  }
		  }
		   		  do{        
				    flag = 0;
				    w = waitpid((*job).pid,&status,WUNTRACED);
				    if(w == -1){app_error("WaitPid"); exit(1);}
				    if(WIFEXITED(status)){
				      flag = 1;
				      in = deletejob(jobs,pid);
				    }else if(WIFSIGNALED(status)){
					    flag = 1;
					    printf("Unable to execute: Killed by signal%d\n", WTERMSIG(status));
				    }else if(WIFSTOPPED(status)){
					    flag = 1;
				      printf("[%d] Suspended\n",(*job).jid);//Suspend process on ctrl + z 
				      (*job).state = ST;
				    }
			}while(flag != 1);	
		 }
}else{
		printf("%s %s : no such job\n",argv[0],argv[1]); // invalid command
	
}
	      return;
}

/* 
 * waitfg - Block until process pid is no longer the foreground process
 */
void waitfg(pid_t pid)
{
   
   int    status;
   while (wait(&status) != pid) ;
    return;
}

/*****************
 * Signal handlers
 *****************/

/* 
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children, but doesn't wait for any other
 *     currently running children to terminate.  
 */
void sigchld_handler(int sig) 
{
int i,in ;
int k = 0;
pid_t status;

//waits on the pid of the process and delete the jobs once completed
 for(i = 0 ; i < MAXJOBS ; i++){
      status = waitpid(jobs[i].pid,&k,WNOHANG);
       if(status > 0){
         printf("\n[%d]  [%d]  +Done\n",jobs[i].jid,jobs[i].pid);
         in = deletejob(jobs, jobs[i].pid);
	 }
        }
    return;
}

/* 
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.  
 */
void sigint_handler(int sig) 
{ 
	printf("\n%s", prompt);
        fflush(stdout);
   return;
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.  
 */
void sigtstp_handler(int sig) 
{
  int i,fg_flag;
   fg_flag = 0;

for(i= 0 ; i < MAXJOBS ; i++){
	if(jobs[i].state == FG){
	    fg_flag = 1;
	    break;
	}
}
if(fg_flag){
for(i= 0 ; i < MAXJOBS ; i++){
       if(jobs[i].state == FG){
          jobs[i].state == ST;
          if(kill(jobs[i].pid,SIGTSTP)){
		printf("[%d] [%s] Suspended by signal %d\n", jobs[i].jid,jobs[i].cmdline,sig);
		break;
		}
     } 
}
} else {
	printf("\n%s", prompt);
            fflush(stdout);
}

    return;
}

/*********************
 * End signal handlers
 *********************/

/**********************************************
 * Helper routines that manipulatei the job list
 **********************************************/

/* clearjob - Clear the entries in a job struct */
void clearjob(struct job_t *job) {
    job->pid = 0;
    job->jid = 0;
    job->state = UNDEF;
    job->cmdline[0] = '\0';
}

/* initjobs - Initialize the job list */
void initjobs(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
	clearjob(&jobs[i]);
}

/* maxjid - Returns largest allocated job ID */
int maxjid(struct job_t *jobs) 
{
    int i, max=0;

    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].jid > max)
	    max = jobs[i].jid;
    return max;
}

/* addjob - Add a job to the job list */
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline) 
{
    int i;
    
    if (pid < 1)
	return 0;

    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid == 0) {
	    jobs[i].pid = pid;
	    jobs[i].state = state;
	    jobs[i].jid = nextjid++;
	    if (nextjid > MAXJOBS)
		nextjid = 1;
	    strcpy(jobs[i].cmdline, cmdline);
  	    if(verbose){
	        printf("Added job [%d] %d %s\n", jobs[i].jid, jobs[i].pid, jobs[i].cmdline);
            }
            return 1;
	}
    }
    printf("Tried to create too many jobs\n");
    return 0;
}

/* deletejob - Delete a job whose PID=pid from the job list */
int deletejob(struct job_t *jobs, pid_t pid) 
{
    int i;
    if (pid < 1)
	return 0;

    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid == pid) {
	    clearjob(&jobs[i]);
	    nextjid = maxjid(jobs)+1;
	    return 1;
	}
    }
    return 0;
}

/* fgpid - Return PID of current foreground job, 0 if no such job */
pid_t fgpid(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].state == ST)
	    return jobs[i].pid;
    return 0;
}

/* getjobpid  - Find a job (by PID) on the job list */
struct job_t *getjobpid(struct job_t *jobs, pid_t pid) {
    int i;

    if (pid < 1)
	return NULL;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].pid == pid)
	    return &jobs[i];
    return NULL;
}

/* getjobjid  - Find a job (by JID) on the job list */
struct job_t *getjobjid(struct job_t *jobs, int jid) 
{
    int i;

    if (jid < 1)
	return NULL;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].jid == jid){
	    return &jobs[i];
            }
}

/* pid2jid - Map process ID to job ID */
int pid2jid(pid_t pid)
{
    int i;

    if (pid < 1)
        return 0; 
    for (i = 0; i < MAXJOBS; i++)
        if (jobs[i].pid == pid) {
            return jobs[i].jid;
        }
    return 0;
}

 /* end job list helper routines
 ******************************/
/* listjobs - Print the job list */
void listjobs(struct job_t *jobs)
{
    int i;

    for (i = 0; i < MAXJOBS; i++) {
        if (jobs[i].pid != 0) {
            printf("[%d] (%d) ", jobs[i].jid, jobs[i].pid);
            switch (jobs[i].state) {
                case BG: 
                    printf("Running ");
                    break;
                case FG: 
                    printf("Foreground ");
                    break;
                case ST: 
                    printf("Stopped ");
                    break;
                case D:
		    printf("Done ");
		    break;
            default:
                    printf("listjobs: Internal error: job[%d].state=%d ", 
                           i, jobs[i].state);
            }
            printf("%s \n", jobs[i].cmdline);
        }
    }
}

/*
 * usage - print a help message
 */
void usage(void) 
{
    printf("Usage: shell [-hvp]\n");
    printf("   -h   print this message\n");
    printf("   -v   print additional diagnostic information\n");
    printf("   -p   do not emit a command prompt\n");
    exit(1);
}

/*
 * unix_error - unix-style error routine
 */
void unix_error(char *msg)
{
    fprintf(stdout, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

/*
 * app_error - application-style error routine
 */
void app_error(char *msg)
{
    fprintf(stdout, "%s\n", msg);
    exit(1);
}

/*
 * Signal - wrapper for the sigaction function
 */
handler_t *Signal(int signum, handler_t *handler) 
{
    struct sigaction action, old_action;

    action.sa_handler = handler;  
    sigemptyset(&action.sa_mask); /* block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0)
	unix_error("Signal error");
    return (old_action.sa_handler);
}

/*
 * sigquit_handler - The driver program can gracefully terminate the
 *    child shell by sending it a SIGQUIT signal.
 */
void sigquit_handler(int sig) 
{
    printf("Terminating after receipt of SIGQUIT signal\n");
    exit(1);
}

