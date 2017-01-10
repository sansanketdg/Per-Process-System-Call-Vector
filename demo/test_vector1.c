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
	int ret, fd;
	char *vec_name = "Vector55";
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
	fd = open("test.txt", O_RDWR);
	printf("file id is %d\n", fd);
	if(fd < 0) {
		printf("couldn't open...Sorry \n");
		goto out;
	}
	ret = read(fd, buf, 5);
	printf("read bytes: %d\n, read data : \n%s\n",ret, buf);

	memset(buf, 0, 1024);
	strncpy(buf, "Rashmi is writing", 17);
	ret = write(fd, buf, 17);
	printf("write bytes: %d\n, write data : \n%s\n",ret, buf);

	close(fd);
	printf("file closed\n");
	
	printf("link returned : %d\n",link("normal.txt", "normal.protected"));
	printf("unlink returned: %d\n",unlink("normal.protected"));

	printf("Successfully tested Vector 55\n");

out:
	close(fd);
}

