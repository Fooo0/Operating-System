#define __LIBRARY__
#include <unistd.h>
#include <errno.h>  
#include <asm/segment.h>  

char temp[64]={0};


int sys_iam(const char* name)
{
	int length = 0;
	int index;
	while(get_fs_byte(name + length) != '\0')
	{
		length++;
	}
	if(length > 23)
	{
		return -EINVAL;
	}
	//printk("Successful: %d\n",length);
	
	index=0;
	while((temp[index] = get_fs_byte(name + index)) != '\0')
	{
		index++;
	}
	return index;
}

int sys_whoami(char* name,unsigned int size)
{
	int length = 0;
	int index;
	while (temp[length] != '\0')
	{
		length++;
	}
	if (size < length)
	{
		return -EINVAL;
	}

	index=0;
	while(temp[index] != '\0')
	{
		put_fs_byte(temp[index],(name + index));
		index++;
	}
	return index;
}