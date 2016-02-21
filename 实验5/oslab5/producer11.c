#define __LIBRARY__
#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<linux/kernel.h>
#include	<fcntl.h>
#include	<sys/types.h>
#define BUFFERSIZE	10

static _syscall2(sem_t *,sem_open,const char *,name,int,value);
static _syscall1(int,sem_post,sem_t *,sem);
static _syscall1(int,sem_wait,sem_t *,sem);
static _syscall1(int,sem_getvalue,sem_t *,sem);
static _syscall1(int,sem_unlink,const char*,name);

static _syscall3(int,shmget,int,key,int,size,int,shmflag);
static _syscall3(void *,shmat,int,shmid,const void *,shmaddr,int,shmflag);
static _syscall1(int,shmdt,int,shmid);
static _syscall3(int,shmctl,int,shmid,int,shmcmd,struct shmid_ds *,buf);

struct int_buffer
{
	int int_count;
	int head;
	int tail;
	int data[BUFFERSIZE];
};

sem_t *mutex;

int main(void)
{
	char err_desc[255];
	int itemValue = -1;
	char mutex_sem[]= "mutex";
	struct int_buffer * logbuf;
	int shmid_main;
	int mutex_value;

	mutex = sem_open(mutex_sem,1);
	if(mutex == NULL)
	{
		printf("create semaphore mutex error!\n");
		return 1;
	}

	mutex_value=sem_getvalue(mutex);
	if(mutex_value==-1)
	{
		printf("sem_getvalue(mutex) error\n");
		perror(err_desc);
		return 1;
	}
	else
	{
		printf("producer:mutext's value=%d\n",mutex_value);
	}

	shmid_main=shmget(1234,sizeof(struct int_buffer),0);
	if(shmid_main == -1)
	{
		printf("shmget(1234,..) error!\n");
		perror(err_desc);
		return -1;
	}
	
	logbuf=(struct int_buffer *)shmat(shmid_main,NULL,0);

	if((long)logbuf==-1)
	{
		printf("in producer shmat(shmid_main,NULL,0) error!\n");
		perror(err_desc);
		exit(-1);
	}

	while(itemValue<499)
	{
		itemValue++;


		while(logbuf->int_count==10)
			;

		if(sem_wait(mutex)!=0)
		{
			printf("in customer %u,sem_post(empty) error!",getpid());
			perror(err_desc);
			break;
		}

		logbuf->int_count++;
		
		if(sem_post(mutex)!=0)
		{
			printf("in customer %u,sem_post(empty) error!\n",getpid());
			perror(err_desc);
			break;
		}

		logbuf->data[logbuf->head]=itemValue;
		(logbuf->head)++;
		if(logbuf->head>=BUFFERSIZE)
		{
			logbuf->head=0;
		}
		
	}


	if(shmdt(shmid_main)!=0)
	{
		printf("in producer shmdt(logbuf) error!\n");
		perror(err_desc);
	}
		
	return 0;
}
