
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

/*
 * head of list of all vectors or new system call
 */
LIST_HEAD(vec_list_head);

/*
 * Always access this variable by gathering the mutex lock
 * 'list_lock'.
*/
DEFINE_MUTEX(list_lock);
char buffer[MAX_BUFFER_SIZE];

/*
 * This function will traverse through the list of all the vectors
 * registered and store the information of all the vectors in a buffer.
 * This buffer data is then copied to the user_buffer which is passed in
 * as an argument.
 * return 0 if no error
 * return -1 if error
*/
int show_all_vectors(char *user_buffer)
{
	int ret;
	char *current_ptr;
	int element_len;
	char temp_str[8];
	struct struct_vector  *datastructureptr = NULL;

	ret = 0;
	current_ptr = NULL;
	memset(buffer, 0, MAX_BUFFER_SIZE);
	current_ptr = buffer;
	list_for_each_entry(datastructureptr, &vec_list_head, nextVector) {
		if (ret > MAX_BUFFER_SIZE) {
			#ifdef DEBUG
			printk(
		"Information about all vectors exceed the MAX_BUFFER_SIZE.\n");
			#endif
			break;
		}
		element_len = strlen(datastructureptr->vector_name);
		memcpy(current_ptr, datastructureptr->vector_name, element_len);
		current_ptr[element_len] = '\n';
		current_ptr = current_ptr + element_len + 1;
		ret = ret + element_len + 1;

		sprintf(temp_str, "%lu", datastructureptr->vector_id);
		element_len = strlen(temp_str);
		memcpy(current_ptr, temp_str, element_len);
		current_ptr[element_len] = '\n';
		current_ptr = current_ptr + element_len + 1;
		ret = ret + element_len + 1;

		memset(temp_str, 0, 8);
		sprintf(temp_str, "%d", datastructureptr->overfuncs_bitmap);
		element_len = strlen(temp_str);
		memcpy(current_ptr, temp_str, element_len);
		current_ptr[element_len] = '\n';
		current_ptr = current_ptr + element_len + 1;
		ret = ret + element_len + 1;

		#if DEBUG
		printk("Inside show_vectors - vector_id  =  %lu\n",
			datastructureptr->vector_id);
		#endif
	}
	memcpy(user_buffer, buffer, ret);
	return ret;
}
EXPORT_SYMBOL(show_all_vectors);

/*
 * This function will traverse through the list of all the vectors
 * registered and store the information of vector as per the vector
 * address given by argument vector_addr in a buffer.
 * This buffer data is then copied to the user_buffer which is passed
 * as an argument
 * return 0 if no error
 * return -1 if error
*/
int show_my_vector(char *user_buffer, void *vector_addr)
{
	int ret = 0;
	int element_len;
	char *current_ptr;
	char temp_str[8];
	struct struct_vector  *datastructureptr = NULL;
	int flag = 0;

	if (user_buffer == NULL) {
		ret = -EFAULT;
		goto out;
	}

	if (vector_addr == NULL) {
		memcpy(user_buffer, "vector not assigned", 19);
		goto out;
	}

	list_for_each_entry(datastructureptr, &vec_list_head, nextVector) {
		if (datastructureptr->overriddenSyscalls_address ==
			(unsigned long)	vector_addr) {
			#if DEBUG
			printk(KERN_INFO "Found my vector - vector_id  =  %lu\n",
				datastructureptr->vector_id);
			#endif
			flag = 1;
			break;
		}

	}
	if (flag == 0) {
		/*
		* Didn't find any vector for this vector_addr
		*/
		ret = 0;
		goto out;
	}

	current_ptr = user_buffer;
	element_len = strlen(datastructureptr->vector_name);
	memcpy(current_ptr, datastructureptr->vector_name, element_len);
	current_ptr[element_len] = '\n';
	current_ptr = current_ptr + element_len + 1;
	ret = ret + element_len + 1;

	sprintf(temp_str, "%lu", datastructureptr->vector_id);
	element_len = strlen(temp_str);
	memcpy(current_ptr, temp_str, element_len);
	current_ptr[element_len] = '\n';
	current_ptr = current_ptr + element_len + 1;
	ret = ret + element_len + 1;

	memset(temp_str, 0, 8);
	sprintf(temp_str, "%d", datastructureptr->overfuncs_bitmap);
	element_len = strlen(temp_str);
	memcpy(current_ptr, temp_str, element_len);
	current_ptr[element_len] = '\n';
	current_ptr = current_ptr + element_len + 1;
	ret = ret + element_len + 1;

out:
	return ret;
}
EXPORT_SYMBOL(show_my_vector);

/*
 * creates a new vector node in the vector list
 */
int add_vector_address(char *vector_name, unsigned long vector_address,
	struct module *vector_module, unsigned int vec_bitmap)
{
	int ret = 0;
	struct struct_vector *va = NULL;

	#if DEBUG
		printk(KERN_INFO "in add_vector_address\n");
	#endif
	va = (struct struct_vector *)kzalloc(sizeof(struct struct_vector),
		GFP_KERNEL);
	if (va == NULL) {
		#if DEBUG
		printk(KERN_INFO "va couldn't be allocated\n");
		#endif
		ret = -ENOMEM;
		goto out;
	}
	if (IS_ERR(va)) {
		ret = PTR_ERR(va);
		goto out;
	}

	#if DEBUG
	printk(KERN_INFO "before locking\n");
	#endif
	va->vector_id = vec_bitmap;
	memcpy(va->vector_name, vector_name, strlen(vector_name));

	va->overriddenSyscalls_address = vector_address;
	va->refcount = 0;
	va->vector_module = vector_module;
	va->overfuncs_bitmap = vec_bitmap;

	#if DEBUG
	printk(KERN_INFO "assigned all values\n");
	#endif

	INIT_LIST_HEAD(&va->nextVector);
	list_add(&va->nextVector, &vec_list_head);

		#if DEBUG
		printk(KERN_INFO
			"Successully created vector in add_vector_address - %s\n",
			vector_name);
		#endif
out:
	return ret;
}

/*
 * creates a new vector node in the vector list dynamically.
 * Puts refcount = 1 as a process is dynamically assigning
 * assigns the vector to the process as well.
 * return 0 if no error.
 * negative number otherwise.
 */
int add_dynamic_vector_address(char *vector_name, unsigned long
	vector_address, struct module *vector_module, unsigned int vec_bitmap)
{
	int ret = 0;
	struct struct_vector *va = NULL;

	#if DEBUG
		printk(KERN_INFO "in add_dynamic_vector_address\n");
	#endif
	va = (struct struct_vector *)kzalloc(sizeof(struct struct_vector),
		GFP_KERNEL);
	if (va == NULL) {
		#if DEBUG
		printk(KERN_INFO "vector couldn't be allocated\n");
		#endif
		ret = -ENOMEM;
		goto out;
	}
	if (IS_ERR(va)) {
		ret = PTR_ERR(va);
		goto out;
	}

	va->vector_id = vec_bitmap;
	memcpy(va->vector_name, vector_name, strlen(vector_name));

	va->overriddenSyscalls_address = vector_address;
	va->refcount = 1;
	va->vector_module = vector_module;
	va->overfuncs_bitmap = vec_bitmap;


	INIT_LIST_HEAD(&va->nextVector);
	list_add(&va->nextVector, &vec_list_head);

		#if DEBUG
		printk(KERN_INFO
			"Successully created vector in add_vector_address - %s\n",
				vector_name);
		#endif

out:
	return ret;
}

/*
 * traverse the list and find the location of vector_address.
 * remove the vector after checking refcount
 * return 0 if everything alright.
 */
int remove_vector_address(unsigned long vector_address)
{
	int ret = 0;
	int flag = 0;

	struct struct_vector *tmp = NULL;
	struct list_head *pos = NULL, *q = NULL;

	pos = &vec_list_head;
	list_for_each_safe(pos, q, &vec_list_head) {
		tmp = list_entry(pos, struct struct_vector, nextVector);
		if (tmp == NULL) {
			#if DEBUG
			printk(KERN_INFO "Coudn't fetch tmp\n");
			#endif
			ret = -EFAULT;
			goto out;
		}
		if (tmp->overriddenSyscalls_address == vector_address) {
			if (tmp->refcount > 0) {
				ret = -2222;
				goto out;
			}

			#if DEBUG
			printk(KERN_INFO "freeing vector = %lu\n", tmp->vector_id);
			#endif
			flag = 1;
			list_del(pos);
			kfree(tmp);
			break;
		}
	}
	if (flag == 0)
		ret = -EFAULT;
out:
	return ret;
}

/*
 * creates new list of vectors of specifying overridden system calls and
 * returns the address of the head of that list
 * returns 0 if no error.
 * else negative number.
 */
int register_syscall(char *vector_name, unsigned long vector_address,
	struct module *vector_module, unsigned int vec_bitmap)
{
	int ret = 0;

	printk(KERN_INFO "grabbing lock\n");
	mutex_lock(&list_lock);
	ret = add_vector_address(vector_name, vector_address, vector_module,
		vec_bitmap);
	mutex_unlock(&list_lock);

	return ret;
}
EXPORT_SYMBOL(register_syscall);

/*
 * called by dynamic vector creation module to check if any current
 * vector exists in the current list.
 *
 * If it doesnt exist it will return 0.
 * returns address of the list of overridden system calls
 */
unsigned long check_any_current_vector(unsigned int vec_bitmap)
{
	int flag = 0;
	struct struct_vector  *datastructureptr = NULL;
	unsigned long ret = 0;

	printk(KERN_INFO "grabbing lock inside check_any_current_vector\n");
	mutex_lock(&list_lock);
	list_for_each_entry(datastructureptr, &vec_list_head, nextVector) {
		if (datastructureptr->overfuncs_bitmap == vec_bitmap) {
			#if DEBUG
			printk(KERN_INFO "Found my vector - vector_id  =  %lu\n",
				datastructureptr->vector_id);
			#endif
			flag = 1;
			break;
		}

	}
	/*
	 * If I find a vector increment its ref count and return
	 * address of list head
	 */
	if (flag != 0) {
		datastructureptr->refcount = datastructureptr->refcount + 1;
		try_module_get(datastructureptr->vector_module);
		ret = datastructureptr->overriddenSyscalls_address;
	}
	mutex_unlock(&list_lock);

	return ret;
}
EXPORT_SYMBOL(check_any_current_vector);

/*
 * function to register a system call if it doesn't exist based on
 * bitmap value given
 * called from dynamic vector module
 */
int register_dynamic_syscall(char *vector_name, unsigned long
	vector_address, struct module *vector_module, unsigned int vec_bitmap)
{
	int ret = 0;

	printk(KERN_INFO "grabbing lock in register_dynamic_syscall\n");
	mutex_lock(&list_lock);
	ret = add_dynamic_vector_address(vector_name, vector_address,
		vector_module, vec_bitmap);
	mutex_unlock(&list_lock);

	return ret;
}
EXPORT_SYMBOL(register_dynamic_syscall);


/*
 * function to unregister list of system calls
 */
int unregister_syscall(unsigned long vector_address)
{
	int ret = 0;

	mutex_lock(&list_lock);
	ret = remove_vector_address(vector_address);
	mutex_unlock(&list_lock);

	return ret;
}
EXPORT_SYMBOL(unregister_syscall);


/*
 * gets vector address from the list of vectors
 * @vector_name: name of the vector to be assigned
 */
unsigned long get_vector_address(char *vector_name)
{
	unsigned long va = 0;
	int flag = 0;
	struct struct_vector  *datastructureptr = NULL;

	mutex_lock(&list_lock);

	list_for_each_entry(datastructureptr, &vec_list_head, nextVector) {
		if (strcmp(vector_name, datastructureptr->vector_name) == 0) {
			#if DEBUG
			printk(KERN_INFO
			"Found vector in get_vector_address - vector_id  =  %lu\n",
				datastructureptr->vector_id);
			#endif
			flag = 1;
			break;
		}

	}

	if (flag != 0) {
		datastructureptr->refcount = datastructureptr->refcount + 1;
		try_module_get(datastructureptr->vector_module);
		va = datastructureptr->overriddenSyscalls_address;
	}
	mutex_unlock(&list_lock);

	return va;
}
EXPORT_SYMBOL(get_vector_address);

/*
 * gets vector_address given just the id or the bitmap of the vector
 */
unsigned long get_vector_address_from_id(unsigned int bitmap)
{
	unsigned long va = 0;
	int flag = 0;
	struct struct_vector  *datastructureptr = NULL;

	mutex_lock(&list_lock);

	list_for_each_entry(datastructureptr, &vec_list_head, nextVector) {
		if (datastructureptr->overfuncs_bitmap == bitmap) {
			#if DEBUG
			printk(KERN_INFO
	"Found vector in get_vector_address_from_id - vector_id  =  %lu\n",
			datastructureptr->vector_id);
			#endif
			flag = 1;
			break;
		}

	}

	if (flag != 0) {
		datastructureptr->refcount = datastructureptr->refcount + 1;
		try_module_get(datastructureptr->vector_module);
		va = datastructureptr->overriddenSyscalls_address;
	}
	mutex_unlock(&list_lock);

	return va;
}
EXPORT_SYMBOL(get_vector_address_from_id);

/*
 * reduces reference count of the vector
 * @vector_name: name of the vector
 */
int reduce_ref_count(char *vector_name)
{
	int ref_cnt = -1;
	struct struct_vector *datastructureptr = NULL;
	int flag = 0;

	mutex_lock(&list_lock);

	list_for_each_entry(datastructureptr, &vec_list_head, nextVector) {
		if (strcmp(vector_name, datastructureptr->vector_name) == 0) {
			#if DEBUG
			printk(KERN_INFO
			"Found vector in get_vector_address - vector_id  =  %lu\n",
				datastructureptr->vector_id);
			#endif
			flag = 1;
			break;
		}
	}

	if (flag != 0) {
		datastructureptr->refcount = datastructureptr->refcount - 1;
		ref_cnt = datastructureptr->refcount;
		module_put(datastructureptr->vector_module);
	}
	mutex_unlock(&list_lock);

	return ref_cnt;
}
EXPORT_SYMBOL(reduce_ref_count);

/*
* increment_ref_count_from_address() - increment ref count for this
* vector which has vector_address as its overriddenSyscalls_address.
* This function will be called by sys_clone2.c
*/
int increment_ref_count_from_address(unsigned long vector_address)
{
	int ret = -1;
	struct struct_vector *datastructureptr = NULL;
	int flag = 0;

	mutex_lock(&list_lock);

	list_for_each_entry(datastructureptr, &vec_list_head, nextVector) {
		if (vector_address ==
			datastructureptr->overriddenSyscalls_address) {
			#if DEBUG
			printk(KERN_INFO
			"Found vector in increment_ref_count_from_address - vector_id  =  %lu\n",
				datastructureptr->vector_id);
			#endif
			flag = 1;
			break;
		}
	}

	if (flag != 0) {
		ret = 0;
		datastructureptr->refcount = datastructureptr->refcount + 1;
		try_module_get(datastructureptr->vector_module);
	}
	mutex_unlock(&list_lock);
	return ret;

}
EXPORT_SYMBOL(increment_ref_count_from_address);

/*
 * reduces reference count of the vector given its address
 * @vector_address: address of the linked list of overridden system calls
 */
int reduce_ref_count_from_address(unsigned long vector_address)
{
	int ref_cnt = -1;
	struct struct_vector *datastructureptr = NULL;
	int flag = 0;

	mutex_lock(&list_lock);

	list_for_each_entry(datastructureptr, &vec_list_head, nextVector) {
		if (vector_address ==
			datastructureptr->overriddenSyscalls_address) {
			#if DEBUG
			printk(KERN_INFO
			"Found vector in get_vector_address - vector_id  =  %lu\n",
				datastructureptr->vector_id);
			#endif
			flag = 1;
			break;
		}
	}

	if (flag != 0) {
		datastructureptr->refcount = datastructureptr->refcount - 1;
		ref_cnt = datastructureptr->refcount;
		module_put(datastructureptr->vector_module);
	}
	mutex_unlock(&list_lock);

	return ref_cnt;
}
EXPORT_SYMBOL(reduce_ref_count_from_address);
