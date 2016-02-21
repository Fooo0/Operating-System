#define __LIBRARY__
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <asm/segment.h>


char name_iam[24];
int sys_iam(const char * name)
{
//
	int name_len=0;
	char* p=name;
	int i;
	for(;get_fs_byte(name++)!='\0';)
	{
		name_len++;
	}
	printk("the name's length is %d.\n",name_len);
	if(name_len>23)
	{

		//for(i=0;i<23;i++)
		//{
		//	name_iam[i]=get_fs_byte(p++);
		//}

		//name_iam[i]='\0';

		//errno=EINVAL;
		//printk("iam set the errno is %d\n",errno);
		return -EINVAL;
		
	}
	else
	{

		for(i=0;i<name_len;i++)
		{
			name_iam[i]=get_fs_byte(p++);
		}

		name_iam[i]='\0';

		return name_len;
	}
}

int sys_whoami(char * name, unsigned int size)
{
	int i;
	int name_len=0;
	char* p=name;

	for(i=0;name_iam[i]!='\0';i++)
	{
		name_len++;
	}

	printk("whoami return the length %d.\n",name_len);
	if(size<name_len)
	{
		return -EINVAL;
	}
	
	//
	for(i=0;i<size;i++)
	{
		/*
		static inline void put_fs_byte(char val,char *addr)
		{
		__asm__ ("movb %0,%%fs:%1"::"r" (val),"m" (*addr));
		}
		*/
		if(name_iam[i]!='\0')
		{
			put_fs_byte(name_iam[i],name++);
		}
		else
		{
			break;
		}
	}
	return i;
}
