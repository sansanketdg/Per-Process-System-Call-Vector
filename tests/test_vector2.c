#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sched.h>
#include "../ioctl_module.h"

clone_body(void *arg) {
	int ret1 = 0;
	char buf1[1024];
	printf("Child process ID : %d\n", getpid());
	ret1 = open("test_clone", 66, 77);
	int file_desc1 = open("/dev/ioctl_device", 0);
	ret1 = ioctl(file_desc1, IOCTL_SHOW_MINE, buf1);
	printf("buf is \n%s\n", buf1);
	close(file_desc1);
	printf("child return value is: %d\n\n", ret1);
	_exit(0);
}

void main(){
	
	pid_t wpid;
	int status = 0;
	void **stack = NULL;
	char buf[1024];
	char vec_name[] = "Vector4";
	struct pid_n_buf *pid_buf;
	unsigned int bitmap = 1017;
	int ret, fd;
	struct bitmap *user_bitmap = NULL;

	user_bitmap = (struct bitmap *)malloc(sizeof(struct bitmap *));
	if (user_bitmap == NULL) {
		printf("couldn't allocate memory\n");
		goto free_stack;
	}

	pid_buf = (struct pid_n_buf *)malloc(sizeof(struct pid_n_buf));
	pid_buf->pid = getpid();
	pid_buf->user_buffer = buf;
	user_bitmap->overfuncs_bitmap = bitmap;

	fd = open("/dev/ioctl_device", O_RDONLY);
	memset(buf, '\0', 1024);
	printf("calling ioctl to show all\n");
	ret = ioctl(fd, IOCTL_SHOW_ALL, buf);
	if (ret < 0) {
		printf("error getting all vector information\n");
	}
	
	printf("buf is \n%s\n", buf);
	memset(buf, '\0', 1024);
	ret = ioctl(fd, IOCTL_CREATE_FROM_BITMAP, (void *)user_bitmap);
	if (ret < 0) {
		printf("ioctl create vector from bitmap failed\n");
		goto out;
	}
	ret = ioctl(fd, IOCTL_SHOW_MINE, buf);
	printf("buf is \n%s\n", buf);
	memset(buf, 0, 1024);
	/*
	printf("My vec id %d\n", pid_buf->pid);
	ret = ioctl(file_desc, IOCTL_VECTOR_FOR_PID, (void *)pid_buf);
	printf("buf is \n%s\n", pid_buf->user_buffer);
	*/
	memset(buf, 0, 1024);

	int temp = open("/etc/passwd", O_RDWR);
	printf("file id is %d\n", temp);
	if (temp < 0) {
		printf("changing file to /etc/shadow\n");
		temp = open("/etc/shadow", O_RDWR);
	}
	if (temp < 0) {
		printf("changing file to normal.txt \n");
		temp = open("normal.c", O_RDWR | O_APPEND);
	}
	if(temp < 0) {
		printf("still couldn't open...exiting");
		goto out;
	}

	ret = read(temp, buf, 5);
	memset(buf, 0, 1024);
	strncpy(buf, "Rashmi is writing", 17);
	ret = write(temp, buf, 17);
	close(temp);
	printf("ret val %d\nbuf is \n%s\n", ret, buf);
	link("normal.text", "normal.protected");
	unlink("normal.protected");

out:

	if (user_bitmap)
		free(user_bitmap);
free_stack:
	close(fd);
	if(stack != NULL){
		free(stack);
	}

}
