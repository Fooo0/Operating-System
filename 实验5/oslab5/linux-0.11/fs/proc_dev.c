/*
 * linux/fs/proc_dev.c
 * 2015-10-22 melon add for procs system
*/

#include <errno.h>

#include <linux/sched.h>
#include <linux/kernel.h>
 #include <asm/segment.h>
 #include <stdarg.h>
 #include <stddef.h>
 
/*
 #include <stddef.h>
#include <linux/kernel.h>
*/
extern int vsprintf(char * buf, const char * fmt, va_list args);

/*static char psbuf[2048];*/
/*
* sprintk for print the info for psinfo
*/
 int sprintk(char * buf, const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i=vsprintf(buf,fmt,args);
	va_end(args);
	return i;
}

 /*
* psinfo  to show the info of process 
*/
int psinfo(unsigned long * pos, char * buf, int count)
{
	struct task_struct ** p;
	int output_count=0;
	char * psbuf=NULL;
	int chars=0;
	int offset=*pos;
	
	if((psbuf=(char *)malloc(sizeof(char *)*1024))==NULL)
	{
		printk("psinfo - malloc error!\n");
		return -EINVAL;
	}

	chars=sprintk(psbuf,"pid\tstate\tfather\tcounter\tstart_time\n");

	for(p = &LAST_TASK ; p >= &FIRST_TASK ; --p)
	{
		if(*p)
		{
			chars+=sprintk(psbuf+chars,"%d\t%d\t%d\t%d\t%d\n",(*p)->pid,(*p)->state,(*p)->father,(*p)->counter,(*p)->start_time);
		}
	}

	*(psbuf+chars)='\0';

	while(count>0)
	{
		if(offset>chars)
		{
			break;
		}
	
		put_fs_byte(*(psbuf+offset),buf++);
		offset++;
		output_count++;
		count--;
		
	}

	(*pos)+=output_count;

	free(psbuf);
	return output_count;
}

int hdinfo(unsigned long * pos, char * buf, int count)
{
	struct super_block * sb; 
	struct buffer_head * bh;
	int total_blocks;
	int total_inodes;
	int used_blocks=0;
	int free_blocks=0;
	int i,j,z;
	char * p=NULL;
	
	int chars=0;
	int offset=*pos;
	int output_count=0;
	char * hdbuf=NULL;

	sb=get_super(current->root->i_dev);
	total_blocks = sb->s_nzones;
	total_inodes=sb->s_ninodes;
	for(i=0;i<sb->s_zmap_blocks;i++)
	{
		bh=sb->s_zmap[i];
		p=(char*)bh->b_data;
		for(j=0;j<1024;j++)
		{
			for(z=1;z<=8;z++)
			{
				if((used_blocks+free_blocks)>=total_blocks)
					break;
				if( *(p+j) & z)
					used_blocks++;
				else
					free_blocks++;
			}
		}
	}

	hdbuf=(char*)malloc(sizeof(char*)*512);
	chars=sprintk(hdbuf,"s_imap_blocks:%d\ns_zmap_blocks:%d\n",sb->s_imap_blocks,sb->s_zmap_blocks);
	chars+=sprintk(hdbuf+chars,"total_blocks:%d\nfree_blcoks:%d\nused_blocks:%d\ntotal_indoes:%d\n",total_blocks,free_blocks,used_blocks,total_inodes);
	//*(hduf+chars)='\n';

	while(count>0)
	{
		if(offset>chars)
			break;

		put_fs_byte(*(hdbuf+offset),buf++);
		offset++;
		output_count++;
		count--;
	}

	(*pos)+=output_count;
	free(hdbuf);
	return output_count;

}

int proc_read(int dev, unsigned long * pos, char * buf, int count)
 {
 	if(dev==0)
 	{
 		return psinfo(pos,buf,count);
 	}
 	if(dev==1)
 	{
 		return hdinfo(pos,buf,count);
 	}

 	return 0;
 }
