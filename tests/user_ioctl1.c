#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>

#include "../ioctl_module.h"
#include "../overridden_syscalls.h"

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

void display_vectors(char *buf)
{
    char *token,*ptr;
    int bitmap;
    int i = 1;

    token = strtok(buf, "\n");
    while( token != NULL ) 
    {  
    	printf("------------------------------\n");
        switch(i%3){
            case 0:
                //process bitmap
                bitmap = strtol(token, &ptr, 10);

                printf("bit_no: %d\n",bitmap);

                if(bitmap & READ_OVERRIDE) {
                    printf("The vector overrides READ\n");
                }

                if(bitmap & WRITE_OVERRIDE) {
                    printf("The vector overrides WRITE\n");
                }

                if(bitmap & OPEN_OVERRIDE) {
                    printf("The vector overrides OPEN\n");
                }

                if(bitmap & CLOSE_OVERRIDE) {
                    printf("The vector overrides CLOSE\n");
                }

                if(bitmap & LINK_OVERRIDE) {
                    printf("The vector overrides LINK\n");
                }

                if(bitmap & UNLINK_OVERRIDE) {
                    printf("The vector overrides UNLINK\n");
                }

                if(bitmap & MKDIR_OVERRIDE) {
                    printf("The vector overrides MKDIR\n");
                }

                if(bitmap & RMDIR_OVERRIDE) {
                    printf("The vector overrides RMDIR\n");
                }

                if(bitmap & WRITE_LOGGED) {
                    printf("The vector overrides WRITE with WRITE_LOGGED\n");
                }

                if(bitmap & OPEN_BLOCKED) {
                    printf("The vector overrides OPEN with OPEN_BLOCKED\n");
                }
                break;
            case 1:
                //show vector name
                printf("The vector name is : %s\n",token);
                break;
            case 2:
                printf("The vector id is : %s\n", token);
                //shoe vector id
                break;

        }
        token = strtok(NULL, "\n");
        i++;
        //bit_no = token;   
    }
}

/* 
 * Main - Call the ioctl functions 
*/
int main(int argc, char **argv)
{
	int ret = 0;
	int child_id = -1;
	int file_desc, i;
	char *proc = "/dev/ioctl_device";
	char *vector_name;
	char *file_name; 	
	char *buf = NULL;

	printf("pid : %d\n", getpid());

	file_name = (char*)malloc(strlen(proc) + 1);
	memset(file_name, '\0', strlen(proc) + 1);
	memcpy(file_name, proc, strlen(proc));

	vector_name = (char*)malloc(MAX_VECTOR_NAME_LEN);
	memset(vector_name, '\0', MAX_VECTOR_NAME_LEN);
	/*
 	memcpy(vector_name, argv[1], strlen(argv[1]));
 	*/
	
	file_desc = open(file_name, O_RDONLY);
	if (file_desc < 0) {
		printf("Can't open file: %s\n", file_name);
		goto free_out;
	}

	
	printf("calling ioctl to get all vector names \n");
        ret = ioctl_get_all_vectors_info(file_desc, vector_name);
        if(ret < 0) {
                printf("ioctl to get my vector failed\n", file_name);
                goto free_out;
        }

    display_vectors(vector_name);

	close(file_desc);
	
free_out:
	free(file_name);
	free(vector_name);
out:
	return ret;
}
