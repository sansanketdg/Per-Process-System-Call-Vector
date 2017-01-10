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

void main(){
	
	char buf[1024];
	char vec_name[] = "Vector968";
	int ret, fd;

	fd = open("/dev/ioctl_device", O_RDONLY);
	memset(buf, '\0', 1024);

	ret = ioctl(fd, IOCTL_SET, vec_name);
	if (ret < 0) {
		printf("failed to set my vector :%d %d\n", errno, fd);
                perror("ERROR");
		goto out;
	}
		
	ret = ioctl(fd, IOCTL_SHOW_MINE, (void *)buf);
	if (ret < 0) {
		printf("failed to get my vector :%d %d\n", errno, fd);
                perror("ERROR");
		goto out;
	}
	printf("my vector is : %s\n", buf);
	memset(buf, '\0', 1024);
	close(fd);
	
	fd = open("/etc/passwd", O_RDWR);
	printf("file id is %d\n", fd);
	if (fd < 0) {
		printf("changing file to /etc/shadow\n");
		fd = open("/etc/shadow", O_RDWR);
	}
	if (fd < 0) {
		printf("changing file to normal.c \n");
		fd = open("normal.c", O_RDWR | O_APPEND);
	}
	if(fd < 0) {
		printf("still couldn't open...exiting\n");
		goto out;
	}

	ret = read(fd, buf, 5);
	memset(buf, 0, 1024);
	strncpy(buf, "Rashmi is writing", 17);
	ret = write(fd, buf, 17);
	close(fd);
	printf("ret val %d\nbuf is \n%s\n", ret, buf);
	
	printf("mkdir returned : %d\n", mkdir("testDir", 644));
	printf("rmmdir returned :%d\n", rmdir("testDir"));

out:
	close(fd);
}
