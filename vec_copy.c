#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <asm/uaccess.h>	/* for get_user and put_user */
#include <linux/string.h>
#include <linux/mutex.h>
#include <linux/slab.h>

#include "vector_struct.h"

#define EXPORT_SYMTAB
#define AUTHOR "Group 17"
#define DESCRIPTION "\'vector_load_unload\' LoadableKernelModule"
MODULE_LICENSE("GPL");
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION(DESCRIPTION);

/*
 * this head is the beginning of list of all vectors or new system call
 * tables created by different applications.
 */
LIST_HEAD(vec_list_head);
/*
 * this variabe defines the current number of vectors existing in the
 * vector list. Always access this variable by gathering the mutex lock
 * 'list_lock'.
*/
int vector_count;
struct mutex list_lock;
char buffer[MAX_BUFFER_SIZE];

/*
 * This function will traverse through the list of all the vectors
 * registered and store the information of all the vectors in a buffer.
 * This buffer data is then copied to the user_buffer which is passed in * as an argument
 * return 0 if no error
 * return -1 if error
*/
static int show_all_vectors(char *user_buffer)
{

	int ret;
	char *current_ptr;
	int vec_len;
	struct struct_vector  *datastructureptr = NULL;

	ret = 0;
	current_ptr = NULL;

	memset(buffer, 0, MAX_BUFFER_SIZE);
	current_ptr = buffer;

	list_for_each_entry(datastructureptr, &vec_list_head, nextVector) {
		if (ret > MAX_BUFFER_SIZE)
			break;
		vec_len = strlen(datastructureptr->vector_name);
		memcpy(current_ptr, datastructureptr->vector_name, vec_len);
		current_ptr[vec_len] = '\n';
		current_ptr = current_ptr + vec_len + 1;
		ret = ret + vec_len + 1;
		#if DEBUG
		printk(KERN_INFO "Inside show_vectors - vector_id  =  %d\n", datastructureptr->vector_id);
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
 * This buffer data is then copied to the user_buffer which is passed in * as an argument
 * return 0 if no error
 * return -1 if error
*/
static int show_my_vector(char *user_buffer, void *vector_addr)
{
	int ret = 0;
	int vec_len;
	struct struct_vector  *datastructureptr = NULL;
	int flag = 0;

	if (user_buffer == NULL) {
		ret = -EFAULT;
		goto out;
	}

	if (vector_addr == NULL) {
		ret = -EFAULT;
		goto out;
	}

	list_for_each_entry(datastructureptr, &vec_list_head, nextVector) {
		if (datastructureptr->overriddenSyscalls_address == (unsigned long)
			vector_addr){
			#if DEBUG
			printk(KERN_INFO "Found my vector - vector_id  =  %d\n", datastructureptr->vector_id);
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
    vec_len = strlen(datastructureptr->vector_name);
    memcpy(user_buffer, datastructureptr->vector_name, vec_len);
    ret = ret + vec_len;

out:
	return ret;
}

EXPORT_SYMBOL(show_my_vector);

/*
 * add_vector_address() - This function will create a new vector node in
 * the vector list. It initialized the refCount = 0 initially.
 * return 0 if no error.
 * negative number other wise.
 */
static int add_vector_address(char *vector_name, unsigned long vector_address, struct module *vector_module)
{
	int ret = 0;
	struct struct_vector *va = NULL;

	#if DEBUG
		printk(KERN_INFO "in add_vector_address\n");
	#endif
	va = (struct struct_vector *)kzalloc(sizeof(struct struct_vector), GFP_KERNEL);
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
		va->vector_id = ++vector_count;
	memcpy(va->vector_name, vector_name, strlen(vector_name));

	va->overriddenSyscalls_address = vector_address;
		va->refcount = 0;
	va->vector_module = vector_module;

	#if DEBUG
			printk(KERN_INFO "assigned all values\n");
		#endif

	INIT_LIST_HEAD(&va->nextVector);
	list_add(&va->nextVector, &vec_list_head);

		#if DEBUG
		printk(KERN_INFO "Successully created vector in add_vector_address - %s\n", vector_name);
		#endif

out:
	return ret;
}

/*
 * traverse the list and find the location of vector_address.
 * remove the vector from the list of vectors only if reference count is
 * 0.
 * returning -2222 for indication that the vector is in use by some
 * process.
 * Therby failing the unregister system_call_vector call.
 *
 * return 0 if everything alright.
 */
static int remove_vector_address(unsigned long vector_address)
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
			printk(KERN_INFO "freeing vector = %d\n", tmp->vector_id);
			#endif
			flag = 1;
			list_del(pos);
			kfree(tmp);
			break;
		}
	}
	if (flag == 0) {
		ret = -EFAULT;
	}

out:
	return ret;
}

/*
 * This is the function called by any module which wants to add new system_call_vector/table to
 * the list of other vectors. This function name is exported to be accessible to other modules.
 *
 * This will create a new structure adding vector_name, address, struct module pointer of module
 * trying to add the vector and reference count for this vector.
 *
 * struct module* pointer is added because when we want that if some process is using the vector,
 * the module implementing the functions/overriden system calls in vector is not removed. So,
 * we will control the reference count of module by using try_get_module() and module_put() on that
 * struct module* pointer.
 *
 * "Mutex" lock has been taken before adding the vector address to the list to ensure mutual exclusion.
 * We want only one module to use the API at a time.
 *
 * returns 0 if no error.
 * else negative number.
 */
static int register_syscall(char *vector_name, unsigned long vector_address, struct module *vector_module)
{
	int ret = 0;

	mutex_lock(&list_lock);
	ret = add_vector_address(vector_name, vector_address, vector_module);
	mutex_unlock(&list_lock);

	return ret;
}
EXPORT_SYMBOL(register_syscall);


/*
 * This is called by module implementing the vector to remove the vector/table from list of
 * registered vectors. This is also exported.
 *
 * This will fail if the vector is in use by some process and is being tried to remove.
 * Otherwise it will remove the vector and deallocate the memory taken by vector.
 *
 * "Mutex" lock has been taken before removing the vector address from the list to ensure mutual exclusion.
 * We want only one module to use the API at a time.
 *
 * returns 0 if no error.
 * else negative number.
 *
 */
static int unregister_syscall(unsigned long vector_address)
{
	int ret = 0;

	mutex_lock(&list_lock);
	ret = remove_vector_address(vector_address);
	mutex_unlock(&list_lock);

	return ret;
}
EXPORT_SYMBOL(unregister_syscall);


/*
 * get_vector_address() - returns the vector address from the list for
 * vector with name 'vector name'
 * @vector_name - name of vector to be searched in the list
 *
 * This is the exported function will be called by ioctl module which
 * takes in name of vector and asks this module to get the address of
 * vector, so that user process can use the overridden
 * functions implemented by owner of vector module.
 *
 * If the vector is found in the list of registered vectors, the
 * reference count of the module, implementing it, is incremented by
 * calling try_module_get() to make sure that nobody removes
 * the vector as the vector is in use by some process.
 * Also the reference counter field kept in the new_vector structure is
 * incremented.
 *
 * The increment of ref_count is made atomic with the use of Mutex lock
 * so as no addition/removal of any vector is done while searching /
 * travesing through the list.
 *
 * returns vector address if successful otherwise returns 0
 */
static unsigned long get_vector_address(char *vector_name)
{
	unsigned long va = 0;
	int flag = 0;
	struct struct_vector  *datastructureptr = NULL;

	mutex_lock(&list_lock);

	list_for_each_entry(datastructureptr, &vec_list_head, nextVector) {
		if (strcmp(vector_name, datastructureptr->vector_name) == 0) {
			#if DEBUG
			printk(KERN_INFO "Found vector in get_vector_address - vector_id  =  %d\n", datastructureptr->vector_id);
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
 * reduce_ref_count() - reduces the reference-count of the vector as per
 * its name given in the argument.
 * @vector_name - name of the vector which is to be search in the vector * list and whose reference-count is to be reduced.
 * This exported function will be called by ioctl module, to reduce the
 * reference count, of the vector. This will happen when a process no
 * longer wants to use the overridden system call vector or when the
 * process is about to exit.
 * In this case we will do a module_put() on the module implementing the
 * vector, also we will reduce the reference counter field in new_vector
 * structure.
 *
 * The decrement of ref_count is made atomic with the use of Mutex lock
 * so as no addition/removal of any vector is done while searching /
 * travesing through the list.
 *
 * returns negative error or updated reference count(0 or positive) if
 * everything is OK.
 *
 */
static int reduce_ref_count(char *vector_name)
{
	int ref_cnt = -1;
	struct struct_vector *datastructureptr = NULL;
	int flag = 0;

	mutex_lock(&list_lock);

	list_for_each_entry(datastructureptr, &vec_list_head, nextVector) {
		if (strcmp(vector_name, datastructureptr->vector_name) == 0) {
			#if DEBUG
			printk(KERN_INFO "Found vector in get_vector_address - vector_id  =  %d\n", datastructureptr->vector_id);
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
 * This is the initialization function of this module
 */
int init_module(void)
{
	int ret = 0;
	struct struct_vector *first = NULL;
	struct struct_vector *second = NULL;
	struct struct_vector  *datastructureptr = NULL;

	vector_count = 0;
	mutex_init(&list_lock);

	first = (struct struct_vector *)kzalloc(sizeof(struct struct_vector), GFP_KERNEL);
	if (first == NULL) {
		#if DEBUG
			printk(KERN_INFO "Coudn't malloc first vector\n");
		#endif
		ret = -ENOMEM;
		goto out;
	}

	second = (struct struct_vector *)kzalloc(sizeof(struct struct_vector), GFP_KERNEL);
	if (second == NULL) {
		#if DEBUG
			printk(KERN_INFO "Coudn't malloc second vector\n");
		#endif
		ret = -ENOMEM;
		goto kfree_first;
	}

	mutex_lock(&list_lock);
	strcpy(first->vector_name, "Vector1");
	first->refcount = 0;
	first->overfuncs_bitmap = 4;

	first->vector_module = THIS_MODULE;
	ret = add_vector_address(first->vector_name, (unsigned long)&first->vector_name, first->vector_module);
	if (ret != 0) {
		#if DEBUG
		printk(KERN_INFO "Unable to create vector1\n");
		#endif
	}

	#if DEBUG
	printk(KERN_INFO "Successully created first vector\n");
	#endif

	strcpy(second->vector_name, "Vector2");
	second->refcount = 0;
	second->overfuncs_bitmap = 8;

	second->vector_module = THIS_MODULE;
	ret = add_vector_address(second->vector_name, (unsigned long)&second->vector_name, THIS_MODULE);
	if (ret != 0) {
		#if DEBUG
		printk(KERN_INFO "Unable to create vector1\n");
		#endif
	}

	#if DEBUG
	printk(KERN_INFO "Successully created second vector\n");
	#endif

	mutex_unlock(&list_lock);

	#if DEBUG
	list_for_each_entry(datastructureptr, &vec_list_head, nextVector) {
		printk(KERN_INFO "vector_id  =  %d\n", datastructureptr->vector_id);
    }
    #endif

    return ret;

 kfree_first:
	kfree(first);

 out:
	return ret;
}

/*
 * This is the module-cleanup function called at the time of removal of
 * module. It will free up the exisiting vector list from the memory.
 */
void cleanup_module(void)
{
	struct list_head *pos = NULL, *q = NULL;
	struct struct_vector  *tmp = NULL;

    pos = &vec_list_head;
    list_for_each_safe(pos, q, &vec_list_head) {
		tmp = list_entry(pos, struct struct_vector, nextVector);
		if (tmp == NULL) {
			#if DEBUG
			printk(KERN_INFO "Coudn't fetch tmp\n");
			#endif
			goto out;
		}
		if (tmp->refcount == 0) {
			#if DEBUG
			printk(KERN_INFO "freeing vector = %d\n", tmp->vector_id);
			#endif
			list_del(pos);
			kfree(tmp);
		}
	}

out:
	return;
}
