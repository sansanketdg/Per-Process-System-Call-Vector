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
#include "../overridden_syscalls.h"

#define MAX_FILENAME 512
#define MAX_VECTOR_NAME_LEN 256

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

int clone_body(void *arg) {
        int ret1 = 0;
        printf("Child ID : %d\n", getpid());
        ret1 = open("test_clone", 66, 77);
        printf("child's return value for open syscall : %d\n\n", ret1);
        sleep(2);
        _exit(0);
}

int main(int argc, char **argv)
{
        int ret = 0;
        void **stack = NULL;
        int file_desc;
        char *proc = "/dev/ioctl_device";
        char *vector_name;
        char *file_name;

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

        file_desc = open(file_name, O_RDONLY);
        printf("/n***********************************   calling ioctl to get my vector      *****************************\n");
        ret = ioctl_get_my_vector_info(file_desc, vector_name);
        if(ret < 0) 
                goto free_out;
        printf("\n  current vector name is:\n%s\n", vector_name);

        stack = (void*)malloc(65535);
        printf("Calling clone syscall \n");

        ret = clone(&clone_body, stack+8092, SIGCHLD | CLONE_FILES | CLONE_VM|CLONE_SYSCALLS, NULL);
        if(ret < 0) {
                printf("ERROR: CLONE syscall failed\n\n");
        }

        sleep(5);

out:
        return ret;
}