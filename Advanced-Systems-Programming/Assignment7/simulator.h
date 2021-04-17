#include <linux/ioctl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <pthread.h>
#include <errno.h>

#define CHARS 256

typedef struct process_param {
    int  fd_num; 
    char fds[64];
    int  argc;
    char argv[CHARS];
} process_param_t;