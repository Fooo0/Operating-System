/*
 *  linux/kernel/printk.c
 *
 *  (C) 1991  Linus Torvalds
 */

/*
 * When in kernel-mode, we cannot use printf, as fs is liable to
 * point to 'interesting' things. Make a printf with fs-saving, and
 * all is well.
 */
#include <stdarg.h>
#include <stddef.h>
#include <linux/sched.h>
#include <sys/stat.h>
 
#include <linux/kernel.h>

static char buf[1024];
static char logbuf[1024];

extern int vsprintf(char * buf, const char * fmt, va_list args);

int printk(const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i=vsprintf(buf,fmt,args);
	va_end(args);
	__asm__("push %%fs\n\t"
		"push %%ds\n\t"
		"pop %%fs\n\t"
		"pushl %0\n\t"
		"pushl $buf\n\t"
		"pushl $0\n\t"
		"call tty_write\n\t"
		"addl $8,%%esp\n\t"
		"popl %0\n\t"
		"pop %%fs"
		::"r" (i):"ax","cx","dx");
	return i;
}

/*编写可在内核调用的write()*/
int fprintk(int fd, const char *fmt, ...)
{
    va_list args;
    int count;
    struct file * file;
    struct m_inode * inode;
    va_start(args, fmt);
    count=vsprintf(logbuf,fmt,args);
    va_end(args); 
    if(fd <3)    /*如果指定的文件号小于3，则表示输出到stdout\stderr,则直接调用sys_write*/
    {
        __asm__("push %%fs\n\t"
            "push %%ds\n\t"
            "pop %%fs\n\t"
            "pushl %0\n\t"
            "pushl $logbuf\n\t"
            "pushl %1\n\t"
            "call sys_write\n\t"
            "addl $8,%%esp\n\t"
            "popl %0\n\t"
            "pop %%fs"
            ::"r" (count),"r"(fd):"ax","cx","dx");
    }
    else /* 假定>=3的描述符都与文件关联。事实上，还存在很多其它情况，这里并没有考虑。*/
    {
        if(!(file=task[0]->filp[fd]))    /* 从进程0的文件描述符表中得到文件句柄 */
            return 0;
        inode=file->f_inode;    //文件节点

        /* 调用file_write写入到指定的文件中即var/process.log */
        __asm__("push %%fs\n\t"
            	"push %%ds\n\t"
            	"pop %%fs\n\t"
            	"pushl %0\n\t"
            	"pushl $logbuf\n\t"
            	"pushl %1\n\t"
            	"pushl %2\n\t"
            	"call file_write\n\t"
            	"addl $12,%%esp\n\t"
            	"popl %0\n\t"
            	"pop %%fs"
            	::"r" (count),"r"(file),"r"(inode):"ax","cx","dx");
    }
    return count;
}
