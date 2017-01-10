#include <linux/linkage.h>
#include <linux/moduleloader.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/moduleloader.h>
#include <asm/segment.h>
#include <asm/page.h>
#include <linux/uaccess.h>
#include <linux/namei.h>
#include <linux/sched.h>
#include "vector_struct.h"
#define CLONE_SYSCALLS 0x00001000
#define EXPORT_SYMTAB
#define AUTHOR "Group 17"
#define DESCRIPTION "sys clone 2"
MODULE_LICENSE("GPL");
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION(DESCRIPTION);

extern long _do_fork(unsigned long clone_flags,
		  unsigned long stack_start,
		  unsigned long stack_size,
		  int __user *parent_tidptr,
		  int __user *child_tidptr,
		  unsigned long tls,
		  unsigned long vector_address);

asmlinkage extern long (*sysptr)(unsigned long flags, void *child_stack,
			 int *ptid, unsigned long newtls,
			 int *ctid, unsigned long id);
extern unsigned long get_vector_address_from_id(unsigned long vector_id);

asmlinkage long sys_clone2(unsigned long flags, void *child_stack,
			 int *ptid, unsigned long newtls,
			 int *ctid, unsigned long id)
{
	/* dummy syscall: returns 0 for non null, -EINVAL for NULL */
	unsigned long addr = 0;
	struct task_struct *tsk = NULL;
	unsigned long ret = 0;
	int cid;
	
	printk(KERN_INFO "Address sys clone2:%lu\n", addr);

	cid = _do_fork(flags, (unsigned long)child_stack, 0,
	  (int __user *)ptid, (int __user *)ctid, 0);

	tsk = pid_task(find_vpid(cid), PIDTYPE_PID);

	if (tsk != NULL && !(flags & CLONE_SYSCALLS)) {
		ret = get_vector_address_from_id(id);
		if(ret != 0){
			tsk->syscall_vector_info = (void *)ret;
		}
	}

	return cid;
}


static int __init init_sys_clone2(void)
{
	printk(KERN_INFO "installed new sys_clone2 module\n");
	if (sysptr == NULL)
		sysptr = sys_clone2;
	return 0;
}
static void  __exit exit_sys_clone2(void)
{
	if (sysptr != NULL)
		sysptr = NULL;
	printk(KERN_INFO"removed sys_clone2 module\n");
}
module_init(init_sys_clone2);
module_exit(exit_sys_clone2);




