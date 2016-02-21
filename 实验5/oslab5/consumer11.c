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
	int get_count=0;
	
	char err_desc[255];
	char mutex_sem[]= "mutex";
	int itemValue = -1;
	int shmid_main;
	struct int_buffer * logbuf;
	int log = open("pclog.log", O_CREAT|O_TRUNC|O_RDWR, 0666);
	char buflog[255];

	int mutex_value;

	shmid_main=shmget(1234,sizeof(struct int_buffer),0);
	if(shmid_main == -1)
	{
		printf("shmget(1234,..) error!\n");
		perror(err_desc);
		return -1;
	}

	
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


	logbuf=(struct int_buffer *)shmat(shmid_main,NULL,0);
	if((long)logbuf==-1)
	{
		printf("in producer shmat(shmid_main,NULL,0) error!\n");
		perror(err_desc);
		exit(-1);
	}

	while(get_count<500)
	{


		while(logbuf->int_count<=0)
			;
		
		
		itemValue=logbuf->data[logbuf->tail];

		if(sem_wait(mutex)!=0)
		{
			printf("in customer %u,sem_post(empty) error!",getpid());
			perror(err_desc);
			break;
		}

		logbuf->int_count--;

		if(sem_post(mutex)!=0)
		{
			printf("in customer %u,sem_post(empty) error!\n",getpid());
			perror(err_desc);
			break;
		}

		(logbuf->tail)++;
		if(logbuf->tail>=BUFFERSIZE)
		{
			logbuf->tail=0;
		}

		lseek(log,0,SEEK_END);
		sprintf(buflog,"%u:%d\n",getpid(),itemValue);
		write(log,&buflog,sizeof(char)*strlen(buflog));

		get_count++;

		
		
	}

	close(log);

	if(shmdt(shmid_main)!=0)
	{
		printf("in customer shmdt(logbuf) error!\n");
		perror(err_desc);
	}

	if(shmctl(shmid_main,0,0)==-1)
	{
		printf("in customer shmctl(shmid?IPC_RMID?0) error!\n");
		perror(err_desc);
	}

	sem_unlink("mutex");

	return 0;
}
