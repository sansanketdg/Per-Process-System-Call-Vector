#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "../ioctl_module.h"

#define MAX_VECTOR_NAME_LEN 256

int ioctl_create_vector(int fd, void *addr)
{
	int err = 0;
	err = ioctl(fd, IOCTL_CREATE_FROM_BITMAP, (void *)addr);
	if (err < 0) {
		printf("ioctl create vector from bitmap failed\n");
		perror("ERROR");
	}
	else{
		printf("Dynamic vector creation successful\n");
		printf("-----------------------------\n");
	}
	return err;
}
int main()
{
	unsigned int overfuncs_bitmap = 51;
	int fd;
	struct bitmap *bitmap;
	bitmap->overfuncs_bitmap = overfuncs_bitmap;

	fd = open("/dev/ioctl_device", O_RDONLY);	
	if (fd < 0) {
		printf("file not opened\n");
		goto out;
	}
	printf("Calling a dynamic vector creation with bitmap %d\n", overfuncs_bitmap);
	ioctl_create_vector(fd, (void *)bitmap);
out:	
	return 0;
}
