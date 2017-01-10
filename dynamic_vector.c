#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/string.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/spinlock_types.h>
#include <linux/spinlock.h>
#include <linux/rwlock_types.h>
#include "vector_struct.h"
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/export.h>
#include <linux/vmalloc.h>
#include <linux/limits.h>
#include <linux/mm.h>
#include <linux/semaphore.h>

#define EXPORT_SYMTAB
#define AUTHOR "Group 17"
#define DESCRIPTION "\'vector_load_unload\' LoadableKernelModule"

MODULE_LICENSE("GPL");
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION(DESCRIPTION);

unsigned long list_heads[256];
int i;
extern int register_dynamic_syscall(char *vector_name,
	unsigned long vector_address, struct module *vector_module,
	unsigned int vec_bitmap);
extern int unregister_syscall(unsigned long);
extern unsigned long check_any_current_vector(unsigned int bitmap);
extern asmlinkage size_t overriddenRead(int fd, char *buf, size_t count);
extern asmlinkage int overriddenOpen(char *pathname, int flags,
	mode_t mode);
extern asmlinkage int overriddenClose(int fd);
extern asmlinkage int overriddenLink(char __user *oldname,
	char __user *newname);
extern asmlinkage int overriddenUnlink(char __user *pathname);
extern asmlinkage size_t overriddenWrite(int fd, char *buf, size_t count);
extern asmlinkage size_t loggedWrite(int fd, char *buf, size_t count);
extern asmlinkage int blockedOpen(char *pathname, int flags, mode_t mode);
extern asmlinkage int overriddenMkdir(char __user *pathname, mode_t mode);
extern asmlinkage int overriddenRmdir(char __user *pathname);

struct overriddenSyscall *listHead;

static int add_syscall_to_vector(int _NR_syscall, unsigned long funcPtr)
{
	int ret = 0;
	struct overriddenSyscall *syscall = NULL;
	struct overriddenSyscall *temp = NULL;

	syscall = (struct overriddenSyscall *)kmalloc(
		sizeof(struct overriddenSyscall), GFP_KERNEL);
	if (syscall == NULL) {
		ret = -ENOMEM;
		goto out;
	}
	if (IS_ERR(syscall)) {
		ret = PTR_ERR(syscall);
		goto out;
	}
	memset(syscall, 0, sizeof(struct overriddenSyscall));
	syscall->_NR_syscall = _NR_syscall;
	syscall->funcPtr = (sys_call_ptr_t)funcPtr;
	if (listHead == NULL) {
		listHead = syscall;
		goto out;
	}

	temp = listHead;
	while (temp->nextSyscall != NULL)
		temp = temp->nextSyscall;
	temp->nextSyscall = syscall;
out:
	return ret;
}

static void delete_vector(struct overriddenSyscall *listHeads)
{
	struct overriddenSyscall *temp = NULL;
	struct overriddenSyscall *new_head = NULL;

	new_head = listHeads;

	if (listHeads == NULL)
		goto end;

	while (new_head->nextSyscall != NULL) {
		temp = new_head;
		new_head = new_head->nextSyscall;
		kfree(temp);
		temp = NULL;
	}

	kfree(new_head);
	new_head = NULL;
	listHeads = NULL;
	goto end;
end:
	return;
}

int add_vector_from_bitmap(unsigned int bitmap)
{
	unsigned long ret;
	char *temp = NULL, *vector_prefix = NULL;
	struct task_struct *task_structure = NULL;

	ret = check_any_current_vector(bitmap);

	if (ret != 0) {
		#if DEBUG
		printk(KERN_INFO "vector already present\n");
		#endif
		task_structure = get_current();
		task_structure->syscall_vector_info = (void *)ret;
		goto out1;
	}

	temp = (char *)kzalloc(32, GFP_KERNEL);
	if (temp == NULL) {
		printk(KERN_INFO "couldn't allocate temp\n");
		goto out1;
	}
	sprintf(temp, "%d", bitmap);

	vector_prefix = kzalloc(7 + strlen(temp), GFP_KERNEL);
	if (vector_prefix == NULL) {
		printk(KERN_INFO "couldn't allocate vector_prefix\n");
		goto out;
	}

	memcpy(vector_prefix, "Vector", 6);
	vector_prefix = strcat(vector_prefix, temp);

	if (bitmap & READ_OVERRIDE) {
		ret = add_syscall_to_vector(__NR_read,
			(unsigned long)overriddenRead);
		if (ret < 0)
			goto free_vector_prefix;
	}

	if (bitmap & WRITE_OVERRIDE) {
		ret = add_syscall_to_vector(__NR_write,
			(unsigned long)overriddenWrite);
		if (ret < 0)
			goto free_vector_prefix;
	}
	if (bitmap & OPEN_OVERRIDE) {
		ret = add_syscall_to_vector(__NR_open,
			(unsigned long)overriddenOpen);
		if (ret < 0)
			goto free_vector_prefix;
	}

	if (bitmap & CLOSE_OVERRIDE) {
		ret = add_syscall_to_vector(__NR_close,
			(unsigned long)overriddenClose);
		if (ret < 0)
			goto free_vector_prefix;
	}

	if (bitmap & LINK_OVERRIDE) {
		ret = add_syscall_to_vector(__NR_link,
			(unsigned long)overriddenLink);
		if (ret < 0)
			goto free_vector_prefix;
	}
	if (bitmap & UNLINK_OVERRIDE) {
		ret = add_syscall_to_vector(__NR_unlink,
			(unsigned long)overriddenUnlink);
		if (ret < 0)
			goto free_vector_prefix;
	}
	if (bitmap & WRITE_LOGGED) {
	ret = add_syscall_to_vector(__NR_write, (unsigned long)loggedWrite);
		if (ret < 0)
			goto free_vector_prefix;
	}

	if (bitmap & OPEN_BLOCKED) {
		ret = add_syscall_to_vector(__NR_open,
			(unsigned long)blockedOpen);
		if (ret < 0)
			goto free_vector_prefix;
	}

	/*
	*printk("bitmap is for vector given is %d\n", bitmap);
	*/
	try_module_get(THIS_MODULE);
	ret = register_dynamic_syscall(vector_prefix, (unsigned long) listHead, THIS_MODULE, bitmap);

	ret = (unsigned long)listHead;
	task_structure = get_current();
	task_structure->syscall_vector_info = (void *)listHead;

	list_heads[i++] = (unsigned long)listHead;
	listHead = NULL;
	return ret;

free_vector_prefix:
	kfree(vector_prefix);

out:
	kfree(temp);
	delete_vector(listHead);
out1:
	listHead = NULL;
	return ret;

}
EXPORT_SYMBOL(add_vector_from_bitmap);

static int __init init_override(void)
{
	int ret = 0;
	int i = 0;

	for (; i < 256; i++)
		list_heads[i] = 0;
	return ret;
}

/*
 * Calls unregister_syscall API to remove the vector from list of
 * registered vectors.
 * Then calls delete_vector() to clean up the memory.
 */
static void __exit cleanup_override(void)
{
	int j, ret;

	for (j = 0; j < i; j++) {
		ret = unregister_syscall((unsigned long)list_heads[j]);
		if ((struct overriddenSyscall *)(list_heads[j]) != NULL)
			delete_vector(((struct overriddenSyscall *)
					(list_heads[j])));

	}
	i = 0;

}

module_init(init_override);
module_exit(cleanup_override);

