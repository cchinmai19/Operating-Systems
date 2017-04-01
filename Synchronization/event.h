//
//  event.h
//  
//
//

#ifndef event_h
#define event_h

#include <linux/types.h>
#include <linux/wait.h>
#include <linux/list.h>
#include <asm/uaccess.h>
#include <linux/spinlock.h>
#include <asm/current.h>
#include <asm/linkage.h>
#include <linux/cred.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/rwlock_types.h>
#include <linux/uidgid.h>

struct event{
    uid_t UID;
    gid_t GID;
    int UIDFlag;
    /* User Signal Enable Bit */
    int GIDFlag;
    /* Group Signal Enable Bit */
    int eventID;
    struct list_head eventList;
    /* linked list */
    wait_queue_head_t wait_queue;
    /* wait queue of processes waiting on the event.*/
    
};

struct event *getEvent(int eventID);

void doeventinit(void);

asmlinkage long sys_doeventopen(void);
asmlinkage long sys_doeventclose(int eventID);
asmlinkage long sys_doeventwait(int eventID);
asmlinkage long sys_doeventsig(int eventID);
asmlinkage long sys_doeventinfo(int num, int *eventIDs);
asmlinkage long sys_doeventchown(int eventID, uid_t UID, gid_t GID);
asmlinkage long sys_doeventchmod(int eventID, int UIDFlag, int GIDFlag);
asmlinkage long sys_doeventstat(int eventID, uid_t *UID, gid_t *GID, int *UIDFlag, int *GIDFlag);

extern rwlock_t eventLock;
extern struct event e;
extern bool event_initialized;

#endif /* event_h */
