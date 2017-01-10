#ifndef _ASM_X86_VECTOR_STRUCT_H
#define _ASM_X86_VECTOR_STRUCT_H

#include <linux/list.h>
#include <asm/syscall.h>
#include "overridden_syscalls.h"

/* set this flag to 0 if you wish to set debug off */
#define DEBUG 0
#define MAX_VECTOR_NAME_LEN 256
#define MAX_BUFFER_SIZE 1024

struct struct_vector {
	unsigned long vector_id;
	char vector_name[MAX_VECTOR_NAME_LEN];
	unsigned long overriddenSyscalls_address;
	int refcount;
	unsigned int overfuncs_bitmap;
	struct module *vector_module;
	struct list_head nextVector; /* pointer to next vector*/
};

struct overriddenSyscall {
	int _NR_syscall;
	sys_call_ptr_t funcPtr;
	const sys_call_ptr_t *syscallTablePtr;
	struct overriddenSyscall *nextSyscall;
};

int reduce_ref_count_from_address(unsigned long vector_address);
#endif /* _ASM_X86_VECTOR_STRUCT_H */
