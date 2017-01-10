#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>		/* open */
#include <unistd.h>		/* exit */
#include <sys/ioctl.h>		/* ioctl */
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
	char vec_name[] = "Vector3";
	struct pid_n_buf *pid_buf;
	pid_buf = (struct pid_n_buf *)malloc(sizeof(struct pid_n_buf));
	pid_buf->pid = getpid();
	pid_buf->user_buffer = buf;

	int file_desc = open("/dev/ioctl_device", 0);
	int ret = ioctl(file_desc, IOCTL_SHOW_ALL, buf);
	printf("buf is \n%s\n", buf);
	memset(buf, 0, 1024);
	ret = ioctl(file_desc, IOCTL_SET, vec_name);
	ret = ioctl(file_desc, IOCTL_SHOW_MINE, buf);
	printf("buf is \n%s\n", buf);
	memset(buf, 0, 1024);

	printf("My vec id %d\n", pid_buf->pid);
	ret = ioctl(file_desc, IOCTL_VECTOR_FOR_PID, (void *)pid_buf);
	printf("buf is \n%s\n", pid_buf->user_buffer);
	memset(buf, 0, 1024);

	int temp = open("normal.txt", O_RDWR);
	printf("file id is %d\n", temp);
	ret = read(temp, buf,5);
	memset(buf, 0, 1024);
	strncpy(buf, "Rashmi is writing", 17);
	ret = write(temp, buf, 17);
	close(temp);
	printf("ret val %d\nbuf is \n%s\n", ret, buf);
	ret = link("normal.txt", "normal2.txt");
	printf("return value link is %d\n", ret);
	ret = unlink("normal2.txt");
	printf("return value unlink is %d\n", ret);
	ret = mkdir("aboutToDelete", 644);
	printf("return value mkdir is %d\n", ret);
	ret = rmdir("aboutToDelete");
	printf("return value rmdir is %d\n", ret);

	//stack = (void **)malloc(65536);
	//ret = clone(&clone_body, stack+6553, CLONE_PARENT, NULL);

	//wpid = wait(&status);

	ret = ioctl(file_desc, IOCTL_REMOVE, vec_name);
	temp = open("Makefile", 0);
	close(temp);

	ret = ioctl(file_desc, IOCTL_SHOW_MINE, buf);
	printf("buf is \n%s\n", buf);

	close(file_desc);
	if(stack != NULL){
		free(stack);
	}

}
