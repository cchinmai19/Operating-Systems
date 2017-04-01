//
//  test.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>



void doeventopenTest(void);
void doeventcloseTest(void);
void doeventwaitTest(void);
void doeventsigTest(void);
void doeventinfoTest(void);
void doeventchownTest(void);
void doeventchmodTest(void);
void doeventstatTest(void);
void doboundaryTest1(void);
void doboundaryTest2(void);
void doboundaryTest3(void);
void doboundaryTest4(void);
void doprocesschown(void);
void *doopen(void *);
void *doclose(void *);

void main(){
    int t;
    while(1){
        printf("1 : Creates new events\n2 : Destroy the event\n3 : Blocks process\n4 : Unblocks all waiting processes\n5 : event info\n6 : Change own\n7 : Change mode\n8 : event state\n9 : doeventsig while no tasks waiting\n10 : doeventsig while multiple tasks waiting\n11 : multiple events open at once and close\n12 : doeventclose while processes waiting\n13 : Change process own\n0 : exit\n");
        printf("Input command : ");
        scanf("%d",&t);
        switch(t){
            case 1:
                doeventopenTest();
                break;
            case 2:
                doeventcloseTest();
                break;
            case 3:
                doeventwaitTest();
                break;
            case 4:
                doeventsigTest();
                break;
            case 5:
                doeventinfoTest();
                break;
            case 6:
                doeventchownTest();
                break;
            case 7:
                doeventchmodTest();
                break;
            case 8:
                doeventstatTest();
                break;
            case 9:
                doboundaryTest1();
                break;
            case 10:
                doboundaryTest2();
                break;
            case 11:
                doboundaryTest3();
                break;
            case 12:
                doboundaryTest4();
                break;
            case 13:
                doprocesschown();
                break;
            case 0:
            default:
                exit(0);
        }
    }
}

void doprocesschown(){
    int euid, egid;
    printf("Input process effective user ID : ");
    scanf("%d",&euid);
    printf("Input process effective group ID : ");
    scanf("%d",&egid);
    /* change process's euid and egid */
    if(setegid(egid)){
        perror("setegid");
        return;
    }
    if(seteuid(euid)){
        perror("seteuid");
        return;
    }
}
 
/* test syscall 181 */
void doeventopenTest(){
    int n, res, eventID;
    printf("Input the number of events you wanna create : ");
    scanf("%d",&n);
    for(int i=0; i<n; i++){
        eventID = syscall(181);
        if(eventID==-1) printf("doeventopen fail\n");
        else printf("%d eventID : %d\n",i+1,eventID);
    }
}

/* test syscall 182 */
void doeventcloseTest(){
    int eventID, res;
    
    printf("euid : %d, egid : %d\n",geteuid(),getegid());
    
    printf("Input eventID to close : ");
    scanf("%d",&eventID);
    res = syscall(182,eventID);
    if(res==-1) printf("doeventclose fail\n");
    else printf("doeventclose sucess\n");
}

/* test syscall 183 */
void doeventwaitTest(){
    pid_t pid;
    int eventID, res;
    
    printf("euid : %d, egid : %d\n",geteuid(),getegid());
    
    printf("Input eventID : ");
    scanf("%d",&eventID);
    pid = getpid();
    printf("process ID %d\n",pid);
    if(fork()==0){
        res = syscall(183,eventID);
        if(res==-1) printf("doeventwait fail\n");
        else printf("doeventwait sucess\n");
        printf("child exits\n");
        exit(0);
    }else{
        printf("parent continues\n");
    }
}

/* test syscall 184 */
void doeventsigTest(){
    int eventID, res;
    
    printf("euid : %d, egid : %d\n",geteuid(),getegid());
    
    printf("Input eventID to signal : ");
    scanf("%d",&eventID);
    res = syscall(184,eventID);
    if(res==-1) printf("doeventsig fail\n");
    else printf("doeventsig sucess, %d processes are unblocked\n",res);
}

/* test syscall 185 */
void doeventinfoTest(){
    int num;
    int *eventIDs;
    int num2;
    /* use the NULL eventIDs to get the number of active event IDs */
    num = syscall(185,0,NULL);
    printf("the number of active event IDs : %d\n",num);
    /* num2 is the number of integers which the memory pointed to by enventIDs can hold*/
    printf("Input the size of array : ");
    scanf("%d",&num2);
    eventIDs = malloc(sizeof(int)*num2);
    
    num2 = syscall(185,num2,eventIDs);
    /* returns -1 on failure, if num is smaller than the number of active events*/
    if(num2==-1){
        printf("doeventinfo fail\n");
        
    }
    for(int i=0; i<num2; i++){
        printf("event ID: %d\n",eventIDs[i]);
    }
}
/* test syscall 205 */
void doeventchownTest(){
    int res, eventID;
    uid_t UID;
    gid_t GID;
    printf("process effective UID: %d\n",geteuid());
    
    printf("Input eventID : ");
    scanf("%d",&eventID);
    printf("Input UID : ");
    scanf("%d",&UID);
    printf("Input GID : ");
    scanf("%d",&GID);
    res = syscall(205,eventID,UID,GID);
    if(res==-1){
        printf("doeventchown fail\n");
    }else{
        printf("doeventchown success\n");
    }
}
/* test syscall 211 */
void doeventchmodTest(){
    int res, eventID, UIDFlag, GIDFlag;
    
    printf("process effective UID: %d\n",geteuid());
    
    printf("Input eventID : ");
    scanf("%d",&eventID);
    printf("Input UIDFlag : ");
    scanf("%d",&UIDFlag);
    printf("Input GIDFlag : ");
    scanf("%d",&GIDFlag);
    res = syscall(211,eventID,UIDFlag,GIDFlag);
    if(res==-1) printf("doeventchown fail.\n");
    else printf("doeventchown success.\n");
}

/* test syscall 214 */
void doeventstatTest(){
    int res, eventID;
    uid_t *UID;
    gid_t *GID;
    int *UIDFlag, *GIDFlag;
    UID = malloc(sizeof(uid_t));
    GID = malloc(sizeof(gid_t));
    UIDFlag = malloc(sizeof(int));
    GIDFlag = malloc(sizeof(int));
    printf("Input eventID : ");
    scanf("%d",&eventID);
    res = syscall(214, eventID, UID, GID, UIDFlag, GIDFlag);
    if(res==-1) printf("doeventstat fail.\n");
    else printf("eventID: %d \nUID: %d\nGID: %d\nUIDFlag: %d\nGIDFlag: %d\n",eventID,*UID,*GID,*UIDFlag,*GIDFlag);
}
/* test if no tasks waiting when doeventsig() is called */
void doboundaryTest1(){
    int eventID,n;
    printf("create a new event\n");
    eventID = syscall(181);
    if(eventID==-1){
        printf("doeventopen fail\n");
        return;
    }
    printf("no tasks waiting when doeventsig() is called\n");
    n = syscall(184,eventID);
    printf("number of processes being signaled: %d\n",n);
}

/* test multiple tasks waiting when doeventsig() is called */
void doboundaryTest2(){
    int eventID, n, res1, res2, res3;
    printf("create a new event\n");
    eventID = syscall(181);
    if(eventID==-1){
        printf("doeventopen fail\n");
        return;
    }
    if(fork()==0){
        if(fork()==0){
            if(fork()==0){
                res3 = syscall(183,eventID);
                if(res3==-1){
                    printf("doeventwait fail\n");
                    return;
                }
                printf("child3 exits\n");
                exit(0);
            }else{
                res2 = syscall(183,eventID);
                if(res2==-1){
                    printf("doeventwait fail\n");
                    return;
                }
                printf("child2 exits\n");
                exit(0);
            }
        }else{
            res1 = syscall(183,eventID);
            if(res1==-1){
                printf("doeventwait fail\n");
                return;
            }
            printf("child1 exits\n");
            exit(0);
        }
    }else{
        sleep(3);
        n = syscall(184,eventID);
        if(n==-1){
            printf("doeventinfo fail\n");
            return;
        }
        printf("multiple tasks waiting when doeventsig() is called\n");
        printf("number of processes being signaled: %d\n",n);
    }
}

int eventIDB[3];
/* test multiple events open at one time */
void doboundaryTest3(){
    long n = 3;
    pthread_t thread[n];
    int res;
    long i, j, k;
    for(i=0; i<n; i++){
        pthread_create(&thread[i],NULL,doopen,(void*)i);
    }
    for(j=0; j<n; j++){
        pthread_join(thread[j],NULL);
    }
    for(i=0; i<n; i++){
        pthread_create(&thread[i],NULL,doclose,(void*)i);
    }
    for(j=0; j<n; j++){
        pthread_join(thread[j],NULL);
    }
}

void *doopen(void *ptr){
    long i;
    i = (long)ptr;
    eventIDB[i] = syscall(181);
    if(eventIDB[i]==-1){
        printf("doeventopen fail\n");
        pthread_exit(NULL);
    }
    printf("event %d open\n",eventIDB[i]);
}

void *doclose(void *ptr){
    long i, res;
    i = (long)ptr;
    res = syscall(182,eventIDB[i]);
    if(res==-1){
        printf("doeventclose fail\n");
        pthread_exit(NULL);
    }
    printf("event %d close\n",eventIDB[i]);
}

/* test process waiting when doeventclose() is called */
void doboundaryTest4(){
    int eventID, res, n, status;
    printf("create a new event\n");
    eventID = syscall(181);
    if(eventID==-1){
        printf("doeventopen fail\n");
        return;
    }
    
    if(fork()==0){
        res = syscall(183,eventID);
        if(res==-1) printf("doeventwait fail\n");
        printf("child exits\n");
        exit(0);
    }else{
        sleep(3);
        n = syscall(182,eventID);
        if(n==-1){
            printf("doeventclose fail\n");
            return;
        }
        printf("process waiting when doeventclose() is called\n");
        printf("number of processes being signaled: %d\n",n);
    }
}
