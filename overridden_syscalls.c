#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/syscalls.h>
#include <linux/mm.h>
#include <linux/unistd.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/file.h>
#include "vector_struct.h"

#define AUTHOR "Group 17"
#define DESCRIPTION "\'System_calls_new_versions\' LKM"

MODULE_LICENSE("GPL");
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION(DESCRIPTION);

static const sys_call_ptr_t *syscallTablePtr;

int initSyscallTablePtr(void)
{
	int ret = 0;
	struct task_struct *task_structure = get_current();
	struct overriddenSyscall *vec_head = NULL;

	if (task_structure->syscall_vector_info != NULL) {
		vec_head = (struct overriddenSyscall *)task_structure->
		syscall_vector_info;
	if (vec_head == NULL) {
		ret = -EFAULT;
		goto out;
	}
	syscallTablePtr = vec_head->syscallTablePtr;
	}
out:
	return ret;
}
char *getProcessName(void)
{
	struct file *fcmdline = NULL;
	char *name = NULL;
	mm_segment_t fs;
	int pid = current->pid;

	name = kmalloc(4096, GFP_KERNEL);
	if (name == NULL)
		return NULL;
	sprintf(name, "/proc/%d/cmdline", pid);
	fcmdline = filp_open(name, O_RDONLY, 0);
	if (fcmdline != NULL) {
	fs = get_fs();
		set_fs(get_ds());
		fcmdline->f_op->read(fcmdline, name, 4096, &
		fcmdline->f_pos);
			set_fs(fs);
	}
	filp_close(fcmdline, NULL);
	return name;
}

asmlinkage size_t overriddenRead(int fd, char *buf, size_t count)
{
	int ret = 8787;
	char *processName = NULL;
	asmlinkage long (*original_read)(unsigned int, char __user *, size_t);

	processName = getProcessName();
	if (processName == NULL)
		goto out;
	printk(KERN_INFO "%s called system call overriddenRead\n",
		processName);

	if (syscallTablePtr == NULL) {
		ret = initSyscallTablePtr();
		if (ret != 0)
			goto out;
	}
	original_read = (void *) *(syscallTablePtr + __NR_read);
	ret = original_read(fd, buf, count);
	printk(KERN_INFO "Original read returned %d\n", ret);
out:
	if (processName != NULL)
		kfree(processName);
	return ret;
}
EXPORT_SYMBOL(overriddenRead);

asmlinkage int overriddenWrite(int fd, char *buf, size_t count)
{
	int ret = 8787;
	char *processName = NULL;
	asmlinkage long (*original_write)(unsigned int, char __user *,
		size_t);

	processName = getProcessName();
	if (processName == NULL)
		goto out;
	printk(KERN_INFO "%s called system call overriddenWrite\n",
		processName);

	if (syscallTablePtr == NULL) {
		ret = initSyscallTablePtr();
		if (ret != 0)
			goto out;
	}
	original_write = (void *) *(syscallTablePtr + __NR_write);
	ret = original_write(fd, buf, count);
	printk(KERN_INFO "Original write returned %d\n", ret);
out:
	if (processName != NULL)
		kfree(processName);
	return ret;
}
EXPORT_SYMBOL(overriddenWrite);

asmlinkage int overriddenOpen(char *pathname, int flags, mode_t mode)
{
	int ret = 8787;
	asmlinkage long (*original_open)(char *, int, mode_t);
	char *processName = NULL;

	processName = getProcessName();
	if (processName == NULL)
		goto out;
	printk(KERN_INFO "%s called system call overriddenOpen\n",
		processName);

	if (syscallTablePtr == NULL) {
		ret = initSyscallTablePtr();
		if (ret != 0)
			goto out;
	}
	original_open = (void *) *(syscallTablePtr + __NR_open);
	ret = original_open(pathname, flags, mode);
	printk(KERN_INFO "Original open returned %d\n", ret);
out:
	if (processName != NULL)
		kfree(processName);
	return ret;
}
EXPORT_SYMBOL(overriddenOpen);


asmlinkage int overriddenClose(int fd)
{
	int ret = 8787;
	asmlinkage long (*original_close)(int);
	char *processName = NULL;

	processName = getProcessName();
	if (processName == NULL)
		goto out;
	printk(KERN_INFO "%s called system call overriddenClose\n",
		processName);

	if (syscallTablePtr == NULL) {
		ret = initSyscallTablePtr();
		if (ret != 0)
			goto out;
	}
	original_close = (void *) *(syscallTablePtr + __NR_close);
	ret = original_close(fd);
	printk(KERN_INFO "Original close returned %d\n", ret);
out:
	if (processName != NULL)
		kfree(processName);
	return ret;
}
EXPORT_SYMBOL(overriddenClose);


asmlinkage int overriddenLink(char __user *oldname, char __user *newname)
{
	int ret = 8787;
	char *processName = NULL;
		asmlinkage int (*original_link)(char *, char*);

	processName = getProcessName();
	if (processName == NULL)
		goto out;
	printk(KERN_INFO "%s called system call overriddenLink\n",
		processName);

	if (syscallTablePtr == NULL) {
		ret = initSyscallTablePtr();
		if (ret != 0)
			goto out;
	}
	original_link = (void *) *(syscallTablePtr + __NR_link);
	ret = original_link(oldname, newname);
	printk(KERN_INFO "Original link returned %d\n", ret);
out:
	if (processName != NULL)
		kfree(processName);
	return ret;
}
EXPORT_SYMBOL(overriddenLink);


asmlinkage int overriddenUnlink(char __user *pathname)
{
	int ret = 8787;
	asmlinkage int (*original_unlink)(char *);
	char *processName = NULL;

	processName = getProcessName();
	if (processName == NULL)
		goto out;
	printk(KERN_INFO "%s called system call overriddenUnlink\n",
		processName);
	if (strstr(pathname, ".protected") != NULL) {
		printk(KERN_INFO "Can't unlink a .protected file\n");
		goto out;
	}
	if (syscallTablePtr == NULL) {
		ret = initSyscallTablePtr();
		if (ret != 0)
			goto out;
	}
	original_unlink = (void *) *(syscallTablePtr + __NR_unlink);
	ret = original_unlink(pathname);
	printk(KERN_INFO "Original unlink returned %d\n", ret);
out:
	if (processName != NULL)
		kfree(processName);
	return ret;
}
EXPORT_SYMBOL(overriddenUnlink);


asmlinkage  int overriddenMkdir(char *pathname, mode_t mode)
{
	int ret = 8787;
	asmlinkage int (*original_mkdir)(char *, mode_t mode);
	char *processName = NULL;

	processName = getProcessName();
	if (processName == NULL)
		goto out;
	printk(KERN_INFO "%s called system call overriddenMkdir\n",
		processName);

	if (syscallTablePtr == NULL) {
		ret = initSyscallTablePtr();
		if (ret != 0)
			goto out;
	}
	original_mkdir = (void *) *(syscallTablePtr + __NR_mkdir);
	ret = original_mkdir(pathname, mode);
	printk(KERN_INFO "Original mkdir returned %d\n", ret);
out:
	if (processName != NULL)
		kfree(processName);
	return ret;
}
EXPORT_SYMBOL(overriddenMkdir);


asmlinkage  int overriddenRmdir(char *pathname)
{

	int ret = 8787;
	asmlinkage int (*original_rmdir)(char *);
	char *processName = NULL;

	processName = getProcessName();
	if (processName == NULL)
		goto out;
	printk(KERN_INFO "%s called system call overriddenRmdir\n",
		processName);
	if (syscallTablePtr == NULL) {
		ret = initSyscallTablePtr();
		if (ret != 0)
			goto out;
	}
	original_rmdir = (void *) *(syscallTablePtr + __NR_rmdir);
	ret = original_rmdir(pathname);
	printk(KERN_INFO "Original rmdir returned %d\n", ret);
out:
	if (processName != NULL)
		kfree(processName);
	return ret;
}
EXPORT_SYMBOL(overriddenRmdir);


asmlinkage int blockedOpen(char *pathname, int flags, mode_t mode)
{
	int ret = 8787;
	asmlinkage long (*original_open)(char *, int, mode_t);
	char *processName = NULL;

	processName = getProcessName();
	if (processName == NULL)
		goto out;
	printk(KERN_INFO "%s called system call blockedOpen\n", processName);
	if (strstr(processName, "httpd") != NULL && (strstr(pathname,
		"/etc/passwd") != NULL || strstr(pathname, "/etc/shadow")
		!= NULL)) {
		printk(KERN_INFO
	"Sorry, httpd process can't open user account related files file\n");
		ret = -1;
		goto out;
	}
	if (syscallTablePtr == NULL) {
		ret = initSyscallTablePtr();
		if (ret != 0)
			goto out;
	}
	original_open = (void *) *(syscallTablePtr + __NR_open);
	ret = original_open(pathname, flags, mode);
	printk(KERN_INFO "Blocked open returned %d\n", ret);
out:
	if (processName != NULL)
		kfree(processName);
	return ret;
}
EXPORT_SYMBOL(blockedOpen);

asmlinkage size_t loggedWrite(int fd, char *buf, size_t count)
{
	int ret = 8787;
	struct file *f;
	char temp_str[32];
	char *kbuf = NULL, *path = NULL;
	asmlinkage long (*original_write)(unsigned int, char __user *, size_t);
	char *str = NULL;
	char *processName = NULL;

	if (syscallTablePtr == NULL) {
		ret = initSyscallTablePtr();
		if (ret != 0)
			goto out;
	}
	original_write = (void *) *(syscallTablePtr + __NR_write);

	processName = getProcessName();
	if (processName == NULL)
		goto out;
	printk(KERN_INFO "%s called loggedWrite\n", processName);
	if (fd < 3)
		goto write;
	f = fget(fd);
	if (f == NULL)
		goto out;
	kbuf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (kbuf == NULL) {
		ret = -ENOMEM;
		goto out;
	}
	path = d_path(&f->f_path, kbuf, PAGE_SIZE);
	if (strstr(path, ".c")) {
		if  (copy_from_user(kbuf, buf, count) != 0) {
			ret = -1;
			goto out;
		}
		str = kzalloc(PAGE_SIZE, GFP_KERNEL);
		strcpy(str, "\n/*below code was added by the user: ");
		sprintf(temp_str, "%d", get_current()->cred->uid.val);
		strcat(str, temp_str);
		strncat(str, "*/\n\0", 44444);
		if (copy_to_user(buf, str, strlen(str)) != 0) {
			ret = -1;
			goto out;
		}
		ret = original_write(fd, buf, strlen(str));
		if (copy_to_user(buf, kbuf, count) != 0) {
			ret = -1;
			goto out;
		}
	}
write:
	ret = original_write(fd, buf, count);
out:
	if (str != NULL)
		kfree(str);
	if (kbuf != NULL)
		kfree(kbuf);
	if (processName != NULL)
		kfree(processName);
	return ret;
}
EXPORT_SYMBOL(loggedWrite);
