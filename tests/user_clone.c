#include <sys/wait.h>
#include <sys/utsname.h>
#include <string.h>
#include <asm/unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <linux/sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include "../ioctl_module.h"


#define CLONE_SYSCALLS 0x00001000
#define STACK_SIZE (1024 * 1024)
#define MAX_VECTOR_NAME_LEN 256
#define READ_OVERRIDE 1
#define WRITE_OVERRIDE 2
#define OPEN_OVERRIDE 4
#define CLOSE_OVERRIDE 8
#define UNLINK_OVERRIDE 16
#define LINK_OVERRIDE 32

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

int main(int argc, char *argv[])
{
	int error = 0;
	long pid;
	int ret, file_desc;
	char *buf = NULL;
	char *proc = "/dev/ioctl_device";
	char *vector_name;
	char *file_name;
    int temp = 0,bitmap;
    char *bit_no;
    char *token;
	
	vector_name = (char *)malloc(256);
    bit_no = (char *)malloc(256);
    memset(bit_no,'\0',256);
	
    file_desc = open(proc, O_RDONLY);
    if (file_desc < 0) {
            printf("Can't open file: %s\n", file_name);
            goto free_out;
    }

	memset(vector_name, '\0', 256);
	memcpy(vector_name, "Vector62", 8);
    printf("calling ioctl to set vector\n");
    ret = ioctl_set_vector(file_desc, vector_name);
    if(ret < 0) {
            goto free_out;
    }

    printf("process has new vector now\n");

	memset(vector_name, '\0', 256);
    printf("calling ioctl to get my vector name \n");
    ret = ioctl_get_all_vectors_info(file_desc, vector_name);
    if(ret < 0) {
            printf("ioctl to get my vector failed\n", file_name);
            goto free_out;
    }

    printf("Parent vector info is: %s\n", vector_name);

    display_vectors(vector_name);


    pid = syscall(329, SIGCHLD, 0, NULL, 0, NULL, 50);
    if(pid == -1) {
        error = -1;
        goto free_out;
    }

    if(pid == 0){
    	sleep(1);
	printf("child process\n");
	memset(vector_name, '\0', 256);
	ioctl_get_my_vector_info(file_desc, vector_name);
	display_vectors(vector_name);
    	goto free_out;
    }else{
    	wait(NULL);
	}
	printf("pid is: %lu\n", pid);


free_out:
	if (vector_name)
		free(vector_name);
out:
	return error;
}
