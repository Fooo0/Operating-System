#define   __LIBRARY__
#include    <semaphore.h> 
#include    <stdio.h> 
#include    <stdlib.h> 
#include    <unistd.h> 
#include    <linux/kernel.h> 
#include    <fcntl.h> 
#include    <linux/types.h> 
#include    <sys/stat.h> 
#include    <sys/types.h> 
#define BUFFERSIZE    10


int main(void)
{
    sem_t *empty;
    sem_t *full;
    sem_t *mutex;

    char empty_sem[]= "empty";
    char full_sem[]= "full";
    char mutex_sem[]="mutex";

    int in = open("pc.log", O_CREAT|O_RDWR, 0666);
    int of = open("pc.log",O_CREAT|O_RDWR,0666);
    char buflog[255];

    int itemValue = -1;
    int fileLen,tmpValue,i,j,k,fullValue;

    ftruncate(in,0);    
    empty = sem_open(empty_sem,O_CREAT,0644,BUFFERSIZE);
    if(empty == SEM_FAILED)    /*fail*/
    {
        printf("create semaphore empty error!\n");
        return 1;
    }

    full = sem_open(full_sem,O_CREAT,0644,0);
    if(full == SEM_FAILED)    /*fail*/
    {
        printf("create semaphore full error!\n");
        return 1;
    }

    mutex = sem_open(mutex_sem,O_CREAT,0644,1);
    if(mutex == SEM_FAILED)    /*fail*/
    {
        printf("create semaphore mutex error!\n");
        return 1;
    }

    /*
    tmpValue=sem_getvalue(empty); 
    printf("now empty's value = %d\n",tmpValue); 

    tmpValue=sem_getvalue(mutex); 
    printf("now mutex's value = %d\n",tmpValue); 

    tmpValue=sem_getvalue(full); 
    printf("now full's value = %d\n",tmpValue);
    */

    if(!fork())
    {
        /*printf("producer process %u !now itemValue=%d\n",getpid(),itemValue);*/
        while(itemValue<499)
        {
            itemValue++;

            if(sem_wait(empty)!=0)    /*fail*/
            {
                printf("in producer sem_wait(empty) error!\n"); 
                break;
            }

            if(sem_wait(mutex)!=0)    /*fail*/
            {
                printf("in producer sem_wait(mutex) error!\n"); 
                break;
            }

            /* fileLen=lseek(in,0,SEEK_END);
            printf("filelen = %d \n",fileLen); */
            lseek(in,0,SEEK_END);
            write(in,&itemValue,sizeof(itemValue));

            /* printf("write %d \n",itemValue); */

            if(sem_post(mutex)!=0)    /*fail*/
            {
                printf("in producer sem_post(mutex) error!\n");
                break;
            }

            if(sem_post(full)!=0)    /*fail*/
            {
                printf("in producer sem_post(full) error!\n");
                break;
            }

        }

        exit(0);
        
    }

    for(i=0; i < 5; i++)
    {
        if(!fork())
        {
            for(k=0;k<100;k++)
            {
                if(sem_wait(full)!=0)
                {
                    printf("in customer %u sem_wait(full) error!\n",getpid());
                    break;
                }
                
                if(sem_wait(mutex)!=0)
                {
                    printf("in customer %u,sem_post(empty) error!",getpid());
                    break;
                }

                lseek(of,0,SEEK_SET);
                read(of,&itemValue,sizeof(itemValue));
                printf("%u:%d\n",getpid(),itemValue);
                /* printf("%d:%d\n",getpid(),itemValue);*/

                fileLen=lseek(in,0,SEEK_END);
                for(j=1;j<(fileLen/sizeof(itemValue));j++)
                {
                    lseek(in,j*sizeof(itemValue),SEEK_SET);
                    read(in,&tmpValue,sizeof(tmpValue));
                    lseek(in,(j-1)*sizeof(itemValue),SEEK_SET);
                    write(in,&tmpValue,sizeof(tmpValue));
                }
                ftruncate(in,fileLen-sizeof(itemValue));

                if(sem_post(mutex)!=0)
                {
                    printf("in customer %u,sem_post(empty) error!\n",getpid());
                    break;
                }

                if(sem_post(empty)!=0)
                {
                    printf("in customer %u,sem_post(empty) error!\n",getpid());
                    break;
                }
                
            }
            exit(0);
        }
    }
    while(waitpid(-1, NULL, WNOHANG) >= 0); 
    
    sem_unlink("empty");
    sem_unlink("full");
    sem_unlink("mutex");
    close(of);
    close(in);

    return 0;
}