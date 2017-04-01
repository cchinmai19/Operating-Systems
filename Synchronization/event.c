//  event.c


#include <linux/event.h>

struct event e;
rwlock_t eventLock;

/* A state indicating whether the global_event has been initialized successfully. */
bool event_initialized;
void display(void);
int get_event_length(struct list_head *head);
/*
 * call when the system is booted
 */
void doeventinit(void){
    
    printk("\n Initialized \n");
    
    /* INitialize the wait queue , evnt queue */
    
    eventLock = __RW_LOCK_UNLOCKED(eventLock);
    INIT_LIST_HEAD(&e.eventList);
    e.eventID = 0;
    init_waitqueue_head(&e.wait_queue);
    event_initialized = true;
}

void display(void){
    struct event *test;
    int i ;
    unsigned long flags;
    i = 0 ;
    
    read_lock_irqsave(&eventLock, flags);
    list_for_each_entry(test, &e.eventList, eventList){
        i = i +1 ;
        printk("event no: %d eventID:  %d\n",i,test->eventID);
        printk("UID: %d GID: %d \n",test->UID,test->GID);
    }
    read_unlock_irqrestore(&eventLock,flags);
}

/* Pass the list as and argument use a pointer pointng to successive
 * entries in the list and increment the length to get number of
 * events in the list.
 */
int get_event_length(struct list_head* head){
    int num_event = 0 ;
    struct list_head *ptr;
    if(head == NULL){
        printk("error get_event_length(): \n");
        return -1;
    }
    
    list_for_each(ptr, head) num_event++;
    return num_event;
}

/* get event from eventID */
struct event *getEvent(int eventID){
    struct event *p;
    list_for_each_entry(p, &e.eventList, eventList){
        if(p->eventID == eventID){
            return p;
        }
    }
    return (struct event *)NULL;
}

/* create a new event and assign an event ID to it
 * Add the new event to event list
 * Return th event id if event was created successfully
 * Return -1 on failure to create event
 */
asmlinkage long sys_doeventopen(void){
    int next_id;
    unsigned long flags;
    struct event *new_event;
    if (event_initialized == false) {
        printk("error sys_doeventopen(): event not initialized\n");
        return -1;
    }
    
    /* allocating the size to the event created */
    new_event = kmalloc(sizeof(struct event),GFP_KERNEL);
    new_event->UID = current->cred->euid.val;
    new_event->GID = current->cred->egid.val;
    new_event->UIDFlag = 1;
    new_event->GIDFlag = 1;
    
    /*lock the event to write */
    write_lock_irqsave(&eventLock, flags);
    
    /* Add new event to the tail of the eventlist*/
    list_add_tail(&(new_event->eventList), &e.eventList);
    
    /* find the next event id and increment it by one and assign a new event id */
    next_id = list_entry((new_event->eventList).prev, struct event,eventList)->eventID;
    
    /* Initialize next eventID */
    new_event->eventID = next_id +1;
    
    /* Initialize wait queue */
    init_waitqueue_head(&(new_event->wait_queue));
    
    /* unlock the event queue */
    write_unlock_irqrestore(&eventLock, flags);
    
    printk("UID: %d ,GID: %d, UIDFlag: %d ,GIDFlag: %d\n",new_event->UID,new_event->GID,new_event->UIDFlag,new_event->GIDFlag);
    printk("Add operation completed\n");
    /*printk("Hello from deveopen!");
    printk("next_id : %d\n", next_id);
    
    
    
    
    display();*/
    return new_event->eventID;
}

asmlinkage long sys_doeventclose(int eventID){
    unsigned long flags;
    struct event * req_event;
    uid_t uid;
    gid_t gid;
    int num_proc_signaled;
    
    /* check if the event is initialized */
    if (event_initialized == false) {
        printk("error sys_doeventclose(): event not initialized\n");
        return -1;
    }
    
    /* get the event and check if the event is NULL */
    
    
    read_lock_irqsave(&eventLock , flags);
    req_event = getEvent(eventID);
    read_unlock_irqrestore(&eventLock, flags);
    
    /*check if the event id is NULL */
    if(req_event == NULL){
        printk("error sys_doeventclose(): event not found. eventID = %d\n",eventID);
        return -1;
    }
    
    printk("UID: %d ,GID: %d , UIDFlag: %d ,GIDFlag: %d\n",req_event->UID,req_event->GID,req_event->UIDFlag,req_event->GIDFlag);
    
    uid = current->cred->euid.val;
    gid = current->cred->egid.val;
    
    if(uid != 0 &&(uid != req_event->UID || req_event->UIDFlag ==0)&& (gid != req_event->GID || req_event->GIDFlag == 0 )){
        printk("sys_doeventclose(): access denied\n");
        return -1 ;
    }
    
    num_proc_signaled = sys_doeventsig(eventID);
    
    /* Remove event from the event list */
    write_lock_irqsave(&eventLock, flags);
    list_del(&req_event->eventList);
    write_unlock_irqrestore(&eventLock,flags);
    
    kfree(req_event); 
    printk("Delete operation completed\n");
    /*display();*/
    return num_proc_signaled;
}


/* Make the event with eventID wait in the queue
 * Return 0 on success
 * Return -1 on failure
 */

asmlinkage long sys_doeventwait(int eventID){
    unsigned long flags;
    uid_t uid;
    gid_t gid;
    wait_queue_t wait;
    struct event * req_event;
    /* check if the event is initialized */
    if (event_initialized == false) {
        printk("error sys_doeventclose(): event not initialized\n");
        return -1;
    }
    
    /* get the event and check if the event is NULL */
   
    
    read_lock_irqsave(&eventLock , flags);
    req_event = getEvent(eventID);
    read_unlock_irqrestore(&eventLock, flags);
    
    /*check if the event id is NULL */
    if(req_event == NULL){
        printk("error sys_doeventclose(): event not found. eventID = %d\n",eventID);
        return -1;
    }
    printk("UID: %d ,GID: %d , UIDFlag: %d,GIDFlag: %d\n",req_event->UID,req_event->GID,req_event->UIDFlag,req_event->GIDFlag);
    
    uid = current->cred->euid.val;
    gid = current->cred->egid.val;
    
    if(uid!=0 &&(uid!=req_event->UID||req_event->UIDFlag==0)&&(gid!=req_event->GID||req_event->GIDFlag==0)){
        printk("sys_doeventsig(): access denied\n");
        return -1 ;
    }
    
    init_wait(&wait);
    
    prepare_to_wait(&(req_event->wait_queue), &wait, TASK_INTERRUPTIBLE);
    schedule();
    finish_wait(&(req_event->wait_queue),&wait);
    printk("wait success.\n");
    
    return 0;
}

/* Wake up all the tasks waiting in the queue with the given eventID
 * REmove all events and no events waiting '
 * Return the number of process signaled
 * Return -1 on failure
 */

asmlinkage long sys_doeventsig(int eventID){
    unsigned long flags;
    int processes_signaled;
    uid_t uid;
    gid_t gid;
    struct event * req_event;
    /* check if the event is initialized */
    if (event_initialized == false) {
        printk("error sys_doeventsig(): event not initialized\n");
        return -1;
    }
    
    /* get the event and check if the event is NULL */

    read_lock_irqsave(&eventLock, flags);
    req_event = getEvent(eventID);
    read_unlock_irqrestore(&eventLock, flags);
    
    /*check if the event id is NULL */
    if(req_event == NULL){
        printk("error sys_doeventsig(): event not found. eventID = %d\n",eventID);
        return -1;
    }
    printk("UID: %d ,GID: %d , UIDFlag: %d ,GIDFlag: %d\n",req_event->UID,req_event->GID,req_event->UIDFlag,req_event->GIDFlag);
    
    uid = current->cred->euid.val;
    gid = current->cred->egid.val;
    
    if(uid != 0 &&(uid != req_event->UID || req_event->UIDFlag ==0)&& (gid != req_event->GID || req_event->GIDFlag == 0 )){
        printk("sys_doeventsig(): access denied\n");
        return -1 ;
    }
    
    /*Unblock all process in the waiting queue and return the no. process signalled */
    
    spin_lock_irqsave(&(req_event->wait_queue.lock),flags);
    
    processes_signaled = get_event_length(&(req_event->wait_queue.task_list));
    
    spin_unlock_irqrestore(&(req_event->wait_queue.lock),flags);
    
    wake_up(&(req_event->wait_queue));
    
    return processes_signaled;
}


/*
 * Fills in the array of integers pointed to by eventIDs with current set of active IDs. num is the
 * number of integers which the memory pointed to by eventIDs can hold. On sucess, doeventinfo() returns the
 * number of active events; otherwise, it returns -1 on failure.
 */
asmlinkage long sys_doeventinfo(int num, int *eventIDs){
    unsigned long flags1, flags2;
    struct event *pos;
    int *sys_eventIDs;
    int count;
    int i;
    if (event_initialized==false) {
        printk("error sys_doeventopen(): event not initialized\n");
        return -1;
    }
    /* reader-weiter spinlocks */
    read_lock_irqsave(&eventLock,flags1);
    /* count event */
    count = get_event_length(&e.eventList);
    read_unlock_irqrestore(&eventLock,flags1);
    
    /* eventIDs can be NULL, in which case, doeventinfo() returns the number of active event IDs. */
    if(!eventIDs){
        return count;
    /* If num is smaller than the number of active event IDs, then -1 should be returned. */
    }else if(num<count){
        printk("Input num is smaller than the number of active eventID\n");
        return -1;
    }
    /* allocate memory to an array to store eventIDs */
    sys_eventIDs = kmalloc(num*sizeof(int),GFP_KERNEL);
    if(!sys_eventIDs){
        kfree(sys_eventIDs);
        printk("sys_doeventinfo(): kmalloc() error\n");
        return -1;
    }
    if(copy_from_user(sys_eventIDs,eventIDs,num*sizeof(int))){
        kfree(sys_eventIDs);
        printk("sys_doeventinfo() : copy_from_user() errot\n");
        return -1;
    }
    
    read_lock_irqsave(&eventLock,flags2);
    i = 0;
    /* don't insert e which eventID is 0 */
    list_for_each_entry(pos, &e.eventList, eventList){
        if(pos->eventID){
            sys_eventIDs[i++] = pos->eventID;
        }
    }
    read_unlock_irqrestore(&eventLock,flags2);

    
    if(copy_to_user(eventIDs, sys_eventIDs, num*sizeof(int))){
        printk("sys_doeventinfo(): copy_to_user() error\n");
        return -1;
    }
    kfree(sys_eventIDs);
    return count;
    
}

/*
 * Change the UID and GID of the event to the specified values; return -1 on failure
 * a process can modify the UID, GID if its effective UID is zero or matches the event's UID
 */
asmlinkage long sys_doeventchown(int eventID, uid_t UID, gid_t GID){
    uid_t uid;
    struct event *cur;
    unsigned long flags;
    if (event_initialized==false) {
        printk("error sys_doeventopen(): event not initialized\n");
        return -1;
    }
    uid = current_euid().val;
    /* reader writer spinlock */
    read_lock_irqsave(&eventLock, flags);
    cur = getEvent(eventID);
    read_unlock_irqrestore(&eventLock, flags);
    
    if(!cur){
        printk("sys_doeventchown: event not found\n");
        return -1;
    }
    
    if(uid && uid!=cur->UID){
        printk("sys_doeventchown: failed to access the event %d\n",eventID);
        return -1;
    }
    printk("UID: %d ,GID: %d , UIDFlag: %d ,GIDFlag: %d\n",cur->UID,cur->GID,cur->UIDFlag,cur->GIDFlag);
    cur->UID = UID;
    cur->GID = GID;
    
    return 0;
}
/*
 * Chang the User Singal Enable Bit to UIDFlag and the Group Signal Enable Bit to GIDFlag;
 * return -1 on failure.
 * a process can modify the User Signal Enable Bit, and Group Signal Enable Bit if its 
 * effective UID is zero or matches the event's UID.
 */
asmlinkage long sys_doeventchmod(int eventID, int UIDFlag, int GIDFlag){
    struct event *cur;
    unsigned long flags;
    uid_t uid;
    if (event_initialized==false) {
        printk("error sys_doeventopen(): event not initialized\n");
        return -1;
    }
    
    /* check if UIDFlag or GIDFlag is valid */
    if(UIDFlag<0||UIDFlag>1){
        printk("sys_doeventchown: UIDFlag %d is not valid\n",UIDFlag);
        return -1;
    }
    if(GIDFlag<0||GIDFlag>1){
        printk("sys_doeventchown: GIDFlag %d is not valid\n",GIDFlag);
        return -1;
    }
    uid = current_euid().val;
    /* reader writer spinlock */
    read_lock_irqsave(&eventLock, flags);
    cur = getEvent(eventID);
    read_unlock_irqrestore(&eventLock, flags);
    
    if(!cur){
        printk("sys_doeventchown: event not found\n");
        return -1;
    }
    if(uid && uid!=cur->UID){
        printk("sys_doeventchown: failed to access the event %d\n",eventID);
        return -1;
    }
    printk("UID: %d ,GID: %d , UIDFlag: %d ,GIDFlag: %d\n",cur->UID,cur->GID,cur->UIDFlag,cur->GIDFlag);
    cur->UIDFlag = UIDFlag;
    cur->GIDFlag = GIDFlag;
    
    return 0;
}
/*
 * Place the UID, GID, User Signal Enable Bit, and Group Signal Enable Bit into the memory
 * pointed to by UID, GID, UIDFlag, and GIDFlag, respectively.
 */
asmlinkage long sys_doeventstat(int eventID, uid_t *UID, gid_t *GID, int *UIDFlag, int *GIDFlag){
    struct event *cur;
    unsigned long flags;
    
    if (event_initialized==false) {
        printk("error sys_doeventopen(): event not initialized\n");
        return -1;
    }
    /* check validity of pointers */
    if(!UID||!GID||!UIDFlag||!GIDFlag){
        printk("sys_doeventstat: pointer is not valid\n");
        return -1;
    }
    
    /* reader writer spinlock */
    read_lock_irqsave(&eventLock, flags);
    cur = getEvent(eventID);
    read_unlock_irqrestore(&eventLock, flags);
    
    if(!cur){
        printk("sys_doeventchown: event not found\n");
        return -1;
    }
    /* copy to user space */
    if(copy_to_user(UID, &(cur->UID), sizeof(uid_t))){
        printk("sys_doeventstat(): copy_to_user() error\n");
        return -1;
    }
    
    if(copy_to_user(GID, &(cur->GID), sizeof(gid_t))){
        printk("sys_doeventstat(): copy_to_user() error\n");
        return -1;
    }
    
    if(copy_to_user(UIDFlag, &(cur->UIDFlag), sizeof(int))){
        printk("sys_doeventstat(): copy_to_user() error\n");
        return -1;
    }
    
    if(copy_to_user(GIDFlag, &(cur->GIDFlag), sizeof(int))){
        printk("sys_doeventstat(): copy_to_user() error\n");
        return -1;
    }
    
    return 0;
}
