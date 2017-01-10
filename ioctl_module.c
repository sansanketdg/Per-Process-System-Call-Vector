/*
 * private data for process is in task_struct->syscall_vector_info
 * it is void *
*/

#include <linux/string.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/mutex.h>
#include <linux/slab.h>

#include "ioctl_module.h"
#include "vector_struct.h"

#define BUF_LEN 256

#define AUTHOR "Group 17"
#define DESCRIPTION "ioctl LKM"
#define DEVICE_NAME "ioctl_device"
#define DEVICE_NUM 23

MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION(DESCRIPTION);
MODULE_LICENSE("GPL");

struct mutex mutexlock;
static long custom_ioctl(struct file *, unsigned int, unsigned long);

/*
 * overriding original implementation of ioctl with our implementation
*/
const struct file_operations fops = {
	.unlocked_ioctl = custom_ioctl,
};

/*
 * function to add vector to the task_struct
 */
int add_to_task_struct(unsigned long vector_struct)
{
	int err, pid;
	struct task_struct *task_structure;

	err = pid = 0;
	task_structure = NULL;

	task_structure = get_current();
	pid = task_structure->pid;
	task_structure->syscall_vector_info = (void *)vector_struct;

	return err;
}

/*
 * function to remove vector from task_struct
 */
int remove_from_task_struct(void)
{
	struct task_struct *task_structure = NULL;
	int err = 0;

	task_structure = get_current();
	task_structure->syscall_vector_info = NULL;

	return err;
}

/*
 * function to check whether current task structure has vector assigned
 * if assigned, reduce reference count
 */
int if_assigned_reduce(void)
{
	struct task_struct *task_structure = NULL;
	char *buf = NULL;
	int err = 0;

	task_structure = get_current();

	if (task_structure->syscall_vector_info != NULL) {
		buf = (char *)kzalloc(BUF_LEN, GFP_KERNEL);
		if (buf == NULL) {
			#if DEBUG
			printk(KERN_INFO "buf couldn't be allocated\n");
			#endif
			err = -ENOMEM;
		goto out;
		}

		show_my_vector(buf, task_structure->syscall_vector_info);
		if (strncmp(buf, "vector not assigned", 19) == 0)
			goto free_vector_name;

		#if DEBUG
			printk(KERN_INFO "reducing refcount for %s\n", buf);
		#endif
		err = reduce_ref_count(buf);

		if (err < 0) {
			#if DEBUG
			printk(KERN_INFO "ref count reduction failed\n");
			#endif
			err = -EINVAL;
			goto free_vector_name;
		}

		#if DEBUG
		printk(KERN_INFO "reduced refcount\n");
		#endif
	}

free_vector_name:
	kfree(buf);

out:
	return err;
}

/*
* validate_bitmap() - Checks if dynamic vector creation ioctl call has
* valid bitmap passed or not.
* @bitmap - passed bitmap from user to be validated.
*/
int validate_bitmap(struct bitmap *bitmap)
{
	int ret = 0;
	/* check if 2 overridden_write syscalls are set*/
	if ((bitmap->overfuncs_bitmap & WRITE_OVERRIDE) && (bitmap->overfuncs_bitmap & WRITE_LOGGED)) {
		ret = -EINVAL;
		#if DEBUG
		printk(KERN_INFO "Error as WRITE_OVERRIDE and  WRITE_LOGGED both set simultaneously\n");
		#endif
		goto out;
	}

	/* check if 2 overridden_write syscalls are set*/
	if ((bitmap->overfuncs_bitmap & OPEN_OVERRIDE) && (bitmap->overfuncs_bitmap & OPEN_BLOCKED)) {
		ret = -EINVAL;
		#if DEBUG
		printk(KERN_INFO "Error as OPEN_OVERRIDE and  OPEN_BLOCKED both set simultaneously\n");
		#endif
		goto out;
	}

	/* check if bitmap value is within allowed range */
	if (bitmap->overfuncs_bitmap > 1023) {
		ret = -EINVAL;
		#if DEBUG
		printk(KERN_INFO "Given bitmap out of range...invalid bitmap value passed to ioctl\n");
		#endif
		goto out;
	}

out:
	return ret;
}

/*
 * custom ioctl definition
 * same as unlocked ioctl
 * @file: pointer to open device(file)
 * @ioctl_request: set or remove vector
 * @ioctl_param: name of the vector or structure containing
 *		 pid and vector_name
 */
static long custom_ioctl(struct file *file, unsigned int ioctl_request,
			 unsigned long ioctl_param)
{
	long err = 0;

	unsigned int vector_length;
	unsigned long vector_address;
	char *vector_name, *user_vector_name;
	struct task_struct *task_structure;
	char *buf;
	struct pid_n_buf *user_pid_buf, *pid_buf;
	struct bitmap *user_bitmap, *kernel_bitmap;

	vector_name = user_vector_name = NULL;
	task_structure = NULL;
	user_pid_buf = pid_buf = NULL;
	user_bitmap = kernel_bitmap = NULL;
	mutex_lock(&mutexlock);

	/*
	 * user_vector_name = (char *)ioctl_param;
	 * vector_length = strlen(user_vector_name);
	 */
	buf = (char *)kzalloc(BUF_LEN, GFP_KERNEL);
	if (buf == NULL) {
		err = -ENOMEM;
		#if DEBUG
			printk(KERN_INFO "error allocating memory for buffer\n");
		#endif
		goto out;
	}

	/*
	 * #if DEBUG
	 * printk(KERN_INFO "checking ioctl_request\n");
	 * #endif
	 */
	switch (ioctl_request) {
		/* set vector address to new vector name*/
	case IOCTL_SET:
		user_vector_name = (char *)ioctl_param;
		vector_length = strlen(user_vector_name);

		vector_name = (char *)kzalloc(vector_length + 1, GFP_KERNEL);
		if (vector_name == NULL) {
			err = -ENOMEM;
			#if DEBUG
			printk(KERN_INFO "error allocating memory for vector_name\n");
			#endif
			goto out;
		}

		if (IS_ERR(vector_name)) {
			err = PTR_ERR(vector_name);
			goto out;
		}

		try_module_get(THIS_MODULE);
		err = copy_from_user(vector_name, user_vector_name,
			vector_length);
		if (err != 0) {
			err = -EINVAL;
			#if DEBUG
			printk(KERN_INFO "error copying vector_name into kernel\n");
			#endif
			module_put(THIS_MODULE);
			goto free_vector_name;
		}
		vector_address = get_vector_address(vector_name);
		if (vector_address == 0) {
			#if DEBUG
			printk(KERN_INFO "error getting vector address\n");
			#endif
			err = -EINVAL;
			module_put(THIS_MODULE);
			goto free_vector_name;
		}

		if_assigned_reduce();
		remove_from_task_struct();
		add_to_task_struct(vector_address);
		module_put(THIS_MODULE);
		break;

		/* remove vector with given name from process use */
	case IOCTL_REMOVE:

		user_vector_name = (char *)ioctl_param;
		vector_length = strlen(user_vector_name);

		vector_name = (char *)kzalloc(vector_length + 1, GFP_KERNEL);
		if (vector_name == NULL) {
			err = -ENOMEM;
			#if DEBUG
			printk(KERN_INFO "error allocating memory for vector_name\n");
			#endif
			goto out;
		}

		if (IS_ERR(vector_name)) {
			err = PTR_ERR(vector_name);
			goto out;
		}

		try_module_get(THIS_MODULE);
		err = copy_from_user(vector_name, user_vector_name,
			vector_length);
		if (err != 0) {
			err = -EINVAL;
			#if DEBUG
			printk(KERN_INFO "error copying vector_name into kernel\n");
			#endif
			module_put(THIS_MODULE);
			goto free_vector_name;
		}
		vector_name[vector_length] = '\0';
		err = reduce_ref_count(vector_name);
		if (err < 0) {
			#if DEBUG
			printk(KERN_INFO "ref count reduction failed\n");
			#endif
			err = -EINVAL;
			module_put(THIS_MODULE);
			goto free_vector_name;
		}

		remove_from_task_struct();

		module_put(THIS_MODULE);
		break;

	case IOCTL_SHOW_ALL:
		try_module_get(THIS_MODULE);
		if (show_all_vectors(buf) == 0)
			memcpy(buf, "no vectors present", 18);

		#if DEBUG
		printk(KERN_INFO "buf: %s\n", buf);
		#endif

		if (copy_to_user((void *)ioctl_param, buf, BUF_LEN)) {
			#if DEBUG
			printk(KERN_INFO "Copy to user of data member failed.\n");
			#endif
			err = -EFAULT;
			module_put(THIS_MODULE);
			goto free_vector_name;
		}
		module_put(THIS_MODULE);
		break;
	case IOCTL_SHOW_MINE:
		try_module_get(THIS_MODULE);

		/* send vector address instead of vector name */
		task_structure = get_current();
		show_my_vector(buf, task_structure->syscall_vector_info);
		#if DEBUG
		printk(KERN_INFO "my vector name: %s\n", buf);
		#endif
		if (copy_to_user((void *)ioctl_param, buf, BUF_LEN)) {
			#if DEBUG
			printk(KERN_INFO "Copy to user of data member failed.\n");
			#endif
			err = -EFAULT;
			module_put(THIS_MODULE);
			goto free_vector_name;
		}

		module_put(THIS_MODULE);
		break;

	case IOCTL_VECTOR_FOR_PID:
		try_module_get(THIS_MODULE);

		pid_buf = (struct pid_n_buf *)kzalloc(
							sizeof(
							struct pid_n_buf),
							GFP_KERNEL);
		user_pid_buf = (struct pid_n_buf *)ioctl_param;
		err = copy_from_user(pid_buf, user_pid_buf,
			sizeof(struct pid_n_buf));
		if (err != 0) {
			err = -EINVAL;
			#if DEBUG
			printk(KERN_INFO "error copying pid_buf into kernel\n");
			#endif
			module_put(THIS_MODULE);
			goto free_vector_name;
		}

		/* find task_struct for a given pid */
		task_structure = pid_task(find_vpid(pid_buf->pid), PIDTYPE_PID);
		show_my_vector(buf, task_structure->syscall_vector_info);

		#if DEBUG
		printk(KERN_INFO "vector name for pid : %d is %s\n",
			pid_buf->pid, buf);
		#endif

		if (copy_to_user((void *)user_pid_buf->user_buffer, buf,
			BUF_LEN)) {
			#if DEBUG
			printk(KERN_INFO "Copy to user of data member failed.\n");
			#endif
			err = -EFAULT;
			module_put(THIS_MODULE);
			goto free_vector_name;
		}

		module_put(THIS_MODULE);
		break;

	/* ioctl to create a new vector from bitmap */
	case IOCTL_CREATE_FROM_BITMAP:
		try_module_get(THIS_MODULE);
		kernel_bitmap = (struct bitmap *)kzalloc(
					sizeof(struct bitmap), GFP_KERNEL);
		user_bitmap = (struct bitmap *)ioctl_param;
		err = copy_from_user(kernel_bitmap, user_bitmap,
			sizeof(struct bitmap));
		if (err != 0) {
			err = -EINVAL;
			#if DEBUG
			printk(KERN_INFO "error copying bitmap into kernel\n");
			#endif
			module_put(THIS_MODULE);
			goto free_vector_name;
		}

		#if DEBUG
		printk(KERN_INFO "bitmap specified in kernel is: %u\n",
			kernel_bitmap->overfuncs_bitmap);
		#endif

		/* validate given bitmap if in range*/
		err = validate_bitmap(kernel_bitmap);
		if (err != 0) {
			#if DEBUG
			printk(KERN_INFO "bitmap invalid\n");
			#endif
			module_put(THIS_MODULE);
			goto free_vector_name;
		}

		/* write code to create and assign vector given a bitmap */

		err = add_vector_from_bitmap(kernel_bitmap->overfuncs_bitmap);

		#if DEBUG
		printk(KERN_INFO "successfully quit add_vector_from_bitmap %lu\n",
				err);
		#endif
		/*
		*err = add_to_task_struct(err);
		*#if DEBUG
		*printk("successfully quit add_to_task_struct\n");
		*#endif
		*/
		module_put(THIS_MODULE);

		break;

	/* some other request received */
	default:
		err = -EINVAL;
		goto free_vector_name;
	}

free_vector_name:
	kfree(buf);
	kfree(vector_name);
	kfree(pid_buf);
	kfree(kernel_bitmap);
out:
	mutex_unlock(&mutexlock);
	return err;
}


/*
 * Here when we initialize the module a new character device with name
 * "ioctl_device" and major number "23" is registered.
 * @ret: negative number on error else 0.
 */
static int __init init_ioctl_module(void)
{
	int ret = 0;

	ret = register_chrdev(DEVICE_NUM, DEVICE_NAME, &fops);
	if (ret < 0) {
		ret = -EINVAL;
		#if DEBUG
		printk(KERN_ALERT "failed to register character device\n");
		#endif
		goto out;
	}
	#if DEBUG
		printk(KERN_INFO "ioctl_module installed successfully\n");
	#endif
	mutex_init(&mutexlock);
out:
	return ret;
}

/*
 * On exiting, the character device is unregistered.
*/
static void __exit exit_ioctl_module(void)
{
	unregister_chrdev(DEVICE_NUM, DEVICE_NAME);
}

module_init(init_ioctl_module);
module_exit(exit_ioctl_module);
