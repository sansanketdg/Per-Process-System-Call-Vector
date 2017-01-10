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
#include "vector_struct.h"

#define AUTHOR "Group 17"
#define DESCRIPTION "\'link_unlink_override\' LKM"
#define MAX_USER_BUF_INPUT_SIZE 512

MODULE_LICENSE("GPL");
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION(DESCRIPTION);

char vector_name[MAX_VECTOR_NAME_LEN] = "Vector55";
struct overriddenSyscall *listHead;

/*
 * exported functions - defined in "reg_unreg" module.
 */
extern int register_syscall(char *vector_name, unsigned long vector_address, struct module *vector_module, unsigned int vec_bitmap);
extern int unregister_syscall(unsigned long vector_address);
extern asmlinkage size_t overriddenRead(int fd, char *buf, size_t count);
extern asmlinkage int overriddenOpen(char *pathname, int flags, mode_t mode);
extern asmlinkage size_t overriddenWrite(int fd, char *buf, size_t count);
extern asmlinkage int overriddenLink(char __user *oldname, char __user *newname);
extern asmlinkage int overriddenUnlink(char __user *pathname);

static int add_syscall_to_vector(int _NR_syscall, unsigned long funcPtr)
{
	int ret = 0;
	struct overriddenSyscall *syscall = NULL;

	struct overriddenSyscall *temp = NULL;

	syscall = (struct overriddenSyscall *)kmalloc(sizeof(struct overriddenSyscall), GFP_KERNEL);
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
    while (temp->nextSyscall != NULL) {
	    temp = temp->nextSyscall;
    }

    temp->nextSyscall = syscall;
out:
	return ret;
}

/*
 * This is a 'cleanup' helper function which deletes the vector,
 * deallocates all the memory
 * allocated earlier while creating the vector.
 * This helper is called when removing the module or when some error
 * occurs.
 */
static void delete_vector(void)
{
	struct overriddenSyscall *temp = NULL;
	struct overriddenSyscall *new_head = NULL;

	new_head = listHead;

	if (listHead == NULL) {
		goto end;
	}

	while (new_head->nextSyscall != NULL) {
		temp = new_head;
		new_head = new_head->nextSyscall;
		kfree(temp);
		temp = NULL;
	}

	kfree(new_head);
	new_head = NULL;
	listHead = NULL;
	goto end;
end:
	return;
}

/*
 * This function is called when the module is initialized. It creates the
 * table of
 * all the system calls we want to override.
 *
 * Then we call the register_syscall API to register our syscall_vector,
 * so that it
 * is made visible in "/proc/syscall_vectors" and can be used by the user
 * process.
 *
 * Returns negative on error and 0 if OK
 */
static int initVector(void)
{
	int ret = 0;
	unsigned int bitmap = 0;

	ret = add_syscall_to_vector(__NR_link, (unsigned long)overriddenLink);
	if (ret < 0) {
		goto out;
	}
	ret = add_syscall_to_vector(__NR_unlink, (unsigned long)overriddenUnlink);
	if (ret < 0) {
		goto out;
	}
	ret = add_syscall_to_vector(__NR_read, (unsigned long)overriddenRead);
	if (ret < 0) {
		goto out;
	}
	ret = add_syscall_to_vector(__NR_open, (unsigned long)overriddenOpen);
	if (ret < 0) {
		goto out;
	}
	ret = add_syscall_to_vector(__NR_write, (unsigned long)overriddenWrite);
	if (ret < 0) {
		goto out;
	}
	bitmap = READ_OVERRIDE + WRITE_OVERRIDE + OPEN_OVERRIDE + LINK_OVERRIDE + UNLINK_OVERRIDE;
	printk("bitmap is for vec1 is %d\n", bitmap);

	ret = register_syscall(vector_name, (unsigned long) listHead, THIS_MODULE, bitmap);
	return ret;

out:
	delete_vector();
	return ret;
}

/*
 * This creates syscall_vector. Called when insmod is done.
 * Returns negative on error. 0 otherwise.
 */
static int __init init_override(void)
{
	int ret = 0;

	ret = initVector();
	if (ret < 0) {
		#if DEBUG
		printk("Error in creating a vector for this module\n");
		#endif
	}
	return ret;
}

/*
 * Calls unregister_syscall API to remove the vector from list of
 * registered vectors.
 * Then calls delete_vector() to clean up the memory.
 */
static void __exit cleanup_override(void)
{
	int ret;

	ret = unregister_syscall((unsigned long)listHead);
	if (listHead != NULL) {
		delete_vector();
	}
}

module_init(init_override);
module_exit(cleanup_override);
