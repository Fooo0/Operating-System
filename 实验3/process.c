#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <time.h>  
#include <sys/times.h>  
#include <sys/wait.h>  
#include <sys/types.h>  
#include <errno.h>

#define HZ  100

void cpuio_bound(int last, int cpu_time, int io_time);


void main()
{
    pid_t c_p1;  
    pid_t c_p2;  
    pid_t c_p3;  
    pid_t c_p4;  
            
	
    if((c_p1 = fork())==0 )
    {
        cpuio_bound( 6, 2, 2); 
		printf( "child1 exit\n");
		exit(0);
    }  
    else if((c_p2 = fork()) == 0)
    {
        cpuio_bound( 7, 3, 0);  
		printf( "child2 exit\n");
		exit(0);
    }  
    else if((c_p3 = fork()) == 0)
    {
        cpuio_bound( 8, 0, 4); 
		printf( "child3 exit\n");
		exit(0);
    }  
    else if((c_p4 = fork()) == 0)
    {
        cpuio_bound( 8, 4, 4);  
		printf( "child4 exit\n");
		exit(0);
    }  
    else if(c_p1 == -1 || c_p2 == -1 || c_p3 == -1 || c_p4 == -1)
    {  
        perror("fork");  
        exit(1);            
    }  
    else
    {  
		while(waitpid(-1, NULL, WNOHANG) >= 0); 
        printf("Now : father (pid: %d) \n", getpid()); 
        printf("The pid of children: \n"); 
        printf("\t Child 1: %d \n", c_p1);  
        printf("\t Child 2: %d \n", c_p2);  
        printf("\t Child 3: %d \n", c_p3);  
        printf("\t Child 4: %d \n", c_p4);  
    }  
}

/*
 * 此函数按照参数占用CPU和I/O时间
 * last: 函数实际占用CPU和I/O的总时间，不含在就绪队列中的时间，>=0是必须的
 * cpu_time: 一次连续占用CPU的时间，>=0是必须的
 * io_time: 一次I/O消耗的时间，>=0是必须的
 * 如果last > cpu_time + io_time，则往复多次占用CPU和I/O
 * 所有时间的单位为秒
 */
void cpuio_bound(int last, int cpu_time, int io_time)
{
    struct tms start_time, current_time;
    clock_t utime, stime;
    int sleep_time;

    while (last > 0)
    {
        /* CPU Burst */
        times(&start_time);
        do
        {
            times(&current_time);
            utime = current_time.tms_utime - start_time.tms_utime;
            stime = current_time.tms_stime - start_time.tms_stime;
        } while ( ( (utime + stime) / HZ )  < cpu_time );
        last -= cpu_time;

        if (last <= 0 )
            break;

        /* IO Burst */
        /* 用sleep(1)模拟1秒钟的I/O操作 */
        sleep_time=0;
        while (sleep_time < io_time)
        {
            sleep(1);
            sleep_time++;
        }
        last -= sleep_time;
    }
}