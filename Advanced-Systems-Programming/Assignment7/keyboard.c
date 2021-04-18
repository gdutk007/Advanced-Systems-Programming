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
#include <sys/stat.h>
#include <sys/mman.h>

#include "simulator.h"

#define MAX_BUFF 2048

// keys buffer
static char buffer[MAX_BUFF];

static int text_file_fd;
static int int_fd;
static int ack_fd; 
static int ctrl_fd;
static int int_mutex_fd;
static int led_buffer_fd;
static int led_mutex_fd; 
static int cond_var_fd;

static int pid;

// process one
static void * interrupt_thread(void * arg);
static void * control_thread(void * arg);
void readFile(char *argv[]);

int init_fds(int argc, char *argv[]);

pthread_cond_t * m_cv;
pthread_mutex_t * m_int_lock;

int main(int argc, char *argv[]){
    setvbuf(stdin,NULL,_IONBF,0);
    setvbuf(stdout,NULL,_IONBF,0);
    pid = getpid();
    printf("%s: argc is %i. Running the %s program.\n",__func__,argc,argv[0]);
    if(argc != 9){
        printf("Please enter a file name for the simulator to parse.\n");
        exit(0);
    }
    printf("%s: argv[%i] is %s \n",__func__,0,argv[0]);
    init_fds(argc, argv); // copying over pipe file descriptors
    printf("%s:fds are the following: %i,%i,%i,%i,%i,%i,%i,%i.\n" ,__func__
                     ,text_file_fd,int_fd,ack_fd,ctrl_fd,led_buffer_fd,led_mutex_fd,cond_var_fd,int_mutex_fd);
    m_cv =        (pthread_cond_t*)mmap(NULL, sizeof(pthread_cond_t),PROT_READ | PROT_WRITE,MAP_SHARED,cond_var_fd, 0);
    m_int_lock = (pthread_mutex_t*)mmap(NULL, sizeof(pthread_mutex_t),PROT_READ | PROT_WRITE,MAP_SHARED,int_mutex_fd, 0);
    int status = 0;
    pid_t child_a, child_b;
    pthread_t mainT;
    int thread;

    //readingfile
    //readFile(argv);
    thread = pthread_create(&mainT,NULL,interrupt_thread,NULL);
    thread = pthread_create(&mainT,NULL,control_thread,NULL);
    pthread_exit(NULL);
    return 0;
}

void readFile(char *argv[]){
    // opening file
    int fd = open(argv[1], O_RDWR);
    if(fd < 0){
            printf("File could not be opened. Failed with: %s\n",strerror(errno));
    }
    char c = '\0';
    ssize_t bytes_read = 0;
    size_t i = 0;
    do{
        bytes_read = read(fd,&c,1);
        if(bytes_read > 0){
           buffer[i++] = c;
        }
    }while(bytes_read != 0 && i < MAX_BUFF);
    if(i >= MAX_BUFF){
        printf("Buffer ran out of space!\n");
        exit(0);
    }
    buffer[i] = '\0';
}

int init_fds(int argc, char *argv[]){
    if(argc < 8 || !argv[1] || !argv[2] || !argv[3] || 
                !argv[4] || !argv[5] || !argv[6] || !argv[7]){
        return -1;
    }
    text_file_fd = strtol(&argv[1][0],(char **)NULL,10);
    int_fd       = strtol(&argv[2][0],(char **)NULL,10);
    ack_fd       = strtol(&argv[3][0],(char **)NULL,10);
    ctrl_fd      = strtol(&argv[4][0],(char **)NULL,10);

    led_buffer_fd  = strtol(&argv[5][0],(char **)NULL,10);
    led_buffer_fd  = strtol(&argv[6][0],(char **)NULL,10);
    cond_var_fd    = strtol(&argv[7][0],(char **)NULL,10);
    int_mutex_fd   = strtol(&argv[8][0],(char **)NULL,10);
    return 0;
}

    /*int thread*/
static void * interrupt_thread(void * arg){
    char c = '\0';
    ssize_t bytes_read = 0;
    size_t i = 0;
    printf("%s: threading pid: %i \n",__func__,pid);
    int signaled = 0;
    do{
        c = '\0';
        printf("about to locking\n");
        pthread_mutex_lock(m_int_lock);
        printf("locking\n");
        bytes_read = read(text_file_fd,&c,1);
        if(bytes_read > 0){
            //printf("%c",c);
            if (write(int_fd, &c, bytes_read) != bytes_read) printf("Failed writing with errno: %s \n",strerror(errno));
            //pthread_cond_wait(m_cv,m_int_lock);
        }
        printf("unlocking\n");
        pthread_mutex_unlock(m_int_lock);
    }while(bytes_read != 0);
    pthread_exit(NULL);
}
    /*control*/
static void * control_thread(void * arg){
    while(1){
        printf("%s: threading pid: %i \n",__func__,pid);
        break;
    }
    pthread_exit(NULL);
}