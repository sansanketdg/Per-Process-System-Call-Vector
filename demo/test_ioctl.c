#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>

#include "../ioctl_module.h"

#define MAX_FILENAME 512
#define MAX_VECTOR_NAME_LEN 256
/* 
 * Functions for the ioctl calls 
 */

int ioctl_remove_vector(int file_desc, char* addr)
{
	int err = 0;

	err = ioctl(file_desc, IOCTL_REMOVE, (void *)addr);
	if (err < 0) {
		printf("ioctl_remove_vector failed: %d %d\n", errno, file_desc);
		perror("ERROR ");
	}
	return err;
}

int ioctl_set_vector(int file_desc, char* addr)
{
	int err = 0;

	err = ioctl(file_desc, IOCTL_SET, (void *)addr);
	if (err < 0) {
		printf("ioctl_set_vector failed:%d %d\n", errno, file_desc);
		perror("ERROR ");
	}

		
	return err;
}

int ioctl_get_my_vector_info(int file_desc, char *addr)
{
	int err = 0;
	
	err = ioctl(file_desc, IOCTL_SHOW_MINE, (void *)addr);
	if (err < 0) {
		printf("ioctl_get_my_vector_info failed:%d %d\n", errno, file_desc);
				perror("ERROR");
	}

	return err;
}

int ioctl_get_all_vectors_info(int fd, char *addr)
{
	int err = 0;
	
	err = ioctl(fd, IOCTL_SHOW_ALL, (void *)addr);
	if (err < 0) {
		printf("ioctl_get_all_vectors_info failed:%d %d\n", errno, fd);
				perror("ERROR");
	}

	return err;
}

int ioctl_create_vector(int fd, void *addr)
{

	int err = 0;
	err = ioctl(fd, IOCTL_CREATE_FROM_BITMAP, (void *)addr);
	if (err < 0) {
		printf("ioctl create vector from bitmap failed");
		perror("ERROR");
	}

	printf("called ioctl successfullly\n");
	return err;
}

int ioctl_vector_for_pid(int fd, void *addr)
{
	int err = 0;
	struct pid_n_buf *pid_buf;
	pid_buf = (struct pid_n_buf *)malloc(sizeof(struct pid_n_buf));
	pid_buf->pid = getpid();
	pid_buf->user_buffer = addr;

	err = ioctl(fd, IOCTL_VECTOR_FOR_PID, (void *)pid_buf);
	if (err < 0) {
		printf("ioctl create vector from bitmap failed");
		perror("ERROR");
	}
	printf("called ioctl successfullly\n");
	free(pid_buf);
	return err;

}

/* 
 * Main - Call the ioctl functions 
*/
int main(int argc, char **argv)
{
	int ret = 0;
	int file_desc, i;
	char *proc = "/dev/ioctl_device";
	char *vector_name;
	char *file_name; 	
	char *buf = NULL;
	unsigned int overfuncs_bitmap = 1017;
	int fd;
	struct bitmap *bitmap;
	bitmap = (struct bitmap*)malloc(sizeof(struct bitmap));
	bitmap->overfuncs_bitmap = overfuncs_bitmap;


	if((argc < 2) || (strlen(argv[1]) > MAX_VECTOR_NAME_LEN)) {
			printf("Vector name must be of maximum length of 32\n");
			goto out;
	}


	printf("pid : %d\n", getpid());

	file_name = (char*)malloc(strlen(proc) + 1);
	if (file_name == NULL) {
		printf("couldn't allocate memory..Sorry\n");
		goto out;
	}
	memset(file_name, '\0', strlen(proc) + 1);
	memcpy(file_name, proc, strlen(proc));

	vector_name = (char*)malloc(MAX_VECTOR_NAME_LEN);
	if (vector_name == NULL) {
		printf("couldn't allocate memory..Sorry\n");
		goto free_out;
	}
	memset(vector_name, '\0', MAX_VECTOR_NAME_LEN);
	memcpy(vector_name, argv[1], strlen(argv[1]));
	
	file_desc = open(file_name, O_RDONLY);

	if (file_desc < 0) {
		printf("Can't open file: %s\n", file_name);
		goto free_out;
	}
	fd = file_desc;

	printf("\n***********************************   calling ioctl to get all vector names      *****************************\n");
		ret = ioctl_get_all_vectors_info(file_desc, vector_name);
		if(ret < 0)
				goto free_out;

	printf("\n  names of vectors \n %s\n", vector_name);

	memset(vector_name, '\0', MAX_VECTOR_NAME_LEN);

		

	printf("/n***********************************   calling ioctl to get my vector      *****************************\n");
		ret = ioctl_get_my_vector_info(file_desc, vector_name);
		if(ret < 0) 
				goto free_out;
	printf("\n  current vector name is:\n%s\n", vector_name);
	
	memset(vector_name, '\0', MAX_VECTOR_NAME_LEN);
		memcpy(vector_name, argv[1], strlen(argv[1]));

	printf("/n***********************************   calling ioctl to set my vector      *****************************\n");
	ret = ioctl_set_vector(file_desc, vector_name);
	if(ret < 0)
		goto free_out;	
	printf("\nProcess has new vector assigned. Will verify it by calling ioctl to get process vector \n");
	
	memset(vector_name, '\0', MAX_VECTOR_NAME_LEN);


	
	printf("\n ***********************************   calling ioctl to get my vector      *****************************\n");
		ret = ioctl_get_my_vector_info(file_desc, vector_name);
		if(ret < 0) 
				goto free_out;
	printf("\n  current vector name is:\n%s\n", vector_name);
	memset(vector_name, '\0', MAX_VECTOR_NAME_LEN);
		
	memcpy(vector_name, argv[1], strlen(argv[1]));


	memset(vector_name, '\0', MAX_VECTOR_NAME_LEN);
printf("/n***********************************   calling ioctl to get vector of some pid     *****************************\n");
		ret = ioctl_vector_for_pid(file_desc, vector_name);
		if(ret < 0) 
				goto free_out;
	printf("\n  received vector name is:\n%s\n", vector_name);
	
	memset(vector_name, '\0', MAX_VECTOR_NAME_LEN);
	memcpy(vector_name, argv[1], strlen(argv[1]));
		
	printf("\n ***********************************   calling ioctl to remove my vector      *****************************\n");
		ret = ioctl_remove_vector(file_desc, vector_name);
		if(ret < 0) 
				goto free_out;
		printf("\nProcess has removed vector. Will verify it by calling ioctl to get process vector \n");
	
	memset(vector_name, '\0', MAX_VECTOR_NAME_LEN);
	
	printf("\n ***********************************   calling ioctl to get my vector      *****************************\n");
		ret = ioctl_get_my_vector_info(file_desc, vector_name);
		if(ret < 0) 
				goto free_out;
	printf("\n  current vector name is:\n%s\n", vector_name);
	memset(vector_name, '\0', MAX_VECTOR_NAME_LEN);
		
	printf("\n *******************************   calling ioctl to create dynamic vector with bitmap 1017   **************************\n");
	ret = ioctl_create_vector(file_desc, (void *)bitmap);
	if(ret < 0)
		goto free_out;
	printf("\nProcess has new vector assigned. Will verify it by calling ioctl to get process vector \n");
	
	memset(vector_name, '\0', MAX_VECTOR_NAME_LEN);
	
	printf("\n ***********************************   calling ioctl to get my vector      *****************************\n");
		ret = ioctl_get_my_vector_info(file_desc, vector_name);
		if(ret < 0) 
				goto free_out;
	printf("\n  current vector name is:\n%s\n", vector_name);
	memset(vector_name, '\0', MAX_VECTOR_NAME_LEN);
		

	printf("\n ******** We are done testing all the ioctls  **************\n");
	close(file_desc);
	
free_out:
	if (bitmap)
		free(bitmap);
	if (file_name)
		free(file_name);
	if (vector_name)
		free(vector_name);
out:
	return ret;
}
