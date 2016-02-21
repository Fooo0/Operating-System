#define __LIBRARY__ 
#include <unistd.h> 
#include <stdarg.h> 
#include <errno.h> 
#include <asm/segment.h> 
#include <asm/system.h> 
#include <string.h> 
#include <linux/kernel.h> 
#include <linux/sched.h>

#define NR_SEMAPHORE 64    // amount of semaphores
#define NR_SEMANAME 255    // length of name
typedef struct semaphore 
{ 
    char sem_name[NR_SEMANAME]; 
    int value; 
    struct task_struct * semp; 
}sem_t;

static sem_t semaphore_list[NR_SEMAPHORE]={{"",0,NULL}};    // initial

/* establish a new semaphore*/ 
sem_t* sys_sem_open(const char * semname, int value) 
{ 
    int i,cursem=-1; 
    char curname[NR_SEMANAME]; 
    char* p=(char *)semname; 
    char c; 
    
    for(i=0;i<NR_SEMANAME;i++)    // get the name 
    { 
        c=get_fs_byte(p++); 
        if(c=='\0')    // end
            break; 
        else 
            curname[i]=c; // one character in name 
    } 
    curname[i]='\0';    // end signal

    // exist or not, 
    // exist : ignore value, and return what exists
    for(i=0; i<NR_SEMAPHORE; i++) 
    { 
        if(strcmp(curname,semaphore_list[i].sem_name)==0)    // exist
        { 
            printk("return semaphore, id is %d,the name is %s,the value is %d\n",i,semaphore_list[i].sem_name,semaphore_list[i].value); 
            return &semaphore_list[i]; 
        } 

    } 
    // find empty place in semaphore_list
    // total 64 allowed : NR_SEMAPHORE in unistd.h
    for(i=0; i<NR_SEMAPHORE; i++) 
    { 
        if(semaphore_list[i].sem_name[0] == '\0')    // empty
        { 
            cursem=i; 
            break; 
        } 
    } 

    if(cursem==-1)    // full
    { 
        printk("now,no blank list,cursem=%d\n",cursem); 
        return NULL; 
    } 
     
    for(i=0;curname[i]!='\0';i++)    // give semaphore name
    { 
        semaphore_list[cursem].sem_name[i]=curname[i]; 
    } 
    semaphore_list[cursem].sem_name[i]='\0';    // end signal
    semaphore_list[cursem].value=value; 
    semaphore_list[cursem].semp=NULL;    // wait

    return &semaphore_list[cursem]; 
}

/* equal to p */
int sys_sem_wait(sem_t * sem) 
{ 
    cli();    // close interuption
    sem->value--; 
    if(sem->value<0)    // not enough resource
    { 
        sleep_on(&(sem->semp));    // wait
    } 
    sti();    // open interuption
    return 0; 
}

/* equal to v */
int sys_sem_post(sem_t * sem) 
{ 
    cli();    // close interuption
    sem->value++;    // give back resource
    if(sem->value<=0)    // some semaphore(s) is/are waiting
    { 
        //wake_up 
        wake_up(&(sem->semp)); 
    } 
    sti();    // open interuption
    return 0; 
}

/* delete semaphore */
int sys_sem_unlink(const char * name) 
{ 
    int i;
    char curname[NR_SEMANAME]; 

    char* p=(char*)name; 
    char c; 
    // get name
    for(i=0;i<NR_SEMANAME;i++) 
    { 
        c=get_fs_byte(p++); 
        if(c=='\0') 
            break; 
        else 
            curname[i]=c;
    } 
    curname[i]='\0';    // end signal
    // exist or not
    for(i=0; i<NR_SEMAPHORE; i++) 
    { 
        if(strcmp(curname,semaphore_list[i].sem_name)==0)    // exist
        { 
        	// delete
            semaphore_list[i].sem_name[0]='\0'; 
            semaphore_list[i].value=0; 
            semaphore_list[i].semp=NULL; 
            return 0; 
        } 
    } 
    // not exist, wrong
    return -1; 
}

/* add a system call for debug */
int sys_sem_getvalue(sem_t * sem) 
{ 
    if(sem != NULL)
        return sem->value; 
    else 
        return -1; 
} 