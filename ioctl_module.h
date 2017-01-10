#ifndef IOCTL_MODULE_H
#define IOCTL_MODULE_H

#include <linux/ioctl.h>

#define IOC_MAGIC 23

#define IOCTL_SET _IOW(IOC_MAGIC, 1, void *)
#define IOCTL_REMOVE _IOW(IOC_MAGIC, 2, void *)
#define IOCTL_SHOW_ALL _IOR(IOC_MAGIC, 3, void *)
#define IOCTL_SHOW_MINE _IOR(IOC_MAGIC, 4, void *)
#define IOCTL_VECTOR_FOR_PID _IOR(IOC_MAGIC, 5, void *)

/* ioctl used to create the vector based on a bitmap */
#define IOCTL_CREATE_FROM_BITMAP _IOW(IOC_MAGIC, 6, void *)
struct pid_n_buf {
	pid_t pid;
	char *user_buffer;
};

struct bitmap {
	unsigned int overfuncs_bitmap;
};

extern int show_all_vectors(char *);
extern int show_my_vector(char *, void *);
extern unsigned long get_vector_address(char *vector_name);
extern int reduce_ref_count(char *vector_name);
extern int add_vector_from_bitmap(unsigned int bitmap);
#endif
