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

// process two

static int int_fd;
static int ack_fd; 
static int ctrl_fd;
static int int_mutex_fd;
static int led_buffer_fd;
static int led_mutex_fd; 
static int cond_var_fd;

static int pid;

void usb_kdb_open();
void input_report_key();
void usb_kbd_irq();
void usb_submit_urb();
void usb_kbd_led();
void usb_kbd_event();
static void * interrupt_thread(void* arg);
static void * control_thread(void* arg);

pthread_cond_t * m_cv;
pthread_mutex_t * m_int_lock;

int init_fds(int argc, char *argv[]);

int main(int argc, char *argv[]){
    setvbuf(stdin,NULL,_IONBF,0);
    setvbuf(stdout,NULL,_IONBF,0);
    pid = getpid();
    printf("%s: argc is %i. Running the %s program.\n",__func__,argc,argv[0]);

    if(argc != 8){
        printf("Missing arguments necessary for simulator.\n");
        exit(0);
    }
    printf("%s: argv[%i] is %s \n",__func__,0,argv[0]);
     init_fds(argc,argv);
     printf("%s:fds are the following: %i,%i,%i,%i,%i,%i,%i.\n" ,__func__
                                         ,int_fd,ack_fd,ctrl_fd,led_buffer_fd,led_mutex_fd,cond_var_fd,int_mutex_fd);
    m_cv = (pthread_cond_t*)mmap(NULL, sizeof(pthread_cond_t),PROT_READ | PROT_WRITE,MAP_SHARED,cond_var_fd, 0);
    m_int_lock = (pthread_mutex_t*)mmap(NULL, sizeof(pthread_mutex_t),PROT_READ | PROT_WRITE,MAP_SHARED,int_mutex_fd, 0);
    usb_kdb_open();
    return 0;
}

int init_fds(int argc, char *argv[]){
    if(argc < 7 || !argv[1] || !argv[2] || !argv[3] || 
                !argv[4] || !argv[5] || !argv[6]){
        return -1;
    }
    int_fd  = strtol(&argv[1][0],(char **)NULL,10);
    ack_fd  = strtol(&argv[2][0],(char **)NULL,10);
    ctrl_fd = strtol(&argv[3][0],(char **)NULL,10);

    led_buffer_fd  = strtol(&argv[4][0],(char **)NULL,10);
    led_buffer_fd  = strtol(&argv[5][0],(char **)NULL,10);
    cond_var_fd    = strtol(&argv[6][0],(char **)NULL,10);
    int_mutex_fd   = strtol(&argv[7][0],(char **)NULL,10);
    return 0;
}

void usb_kdb_open(){
    usb_submit_urb();
}

void input_report_key(){

}

void usb_kbd_irq(){

}

void usb_submit_urb(){
    int status = 0;
    pthread_t mainT;
    int thread;
    thread = pthread_create(&mainT,NULL,interrupt_thread,NULL);
    thread = pthread_create(&mainT,NULL,control_thread  ,NULL);
    pthread_exit(NULL);
}

void usb_kbd_led(){

}

void usb_kbd_event(){

}

static void * interrupt_thread(void * arg){
    char c = '\0';
    ssize_t bytes_read = 0;
    size_t i = 0;
    printf("%s: threading pid: %i \n",__func__,pid);
    do{
        c = '\0';
        bytes_read = read(int_fd,&c,1); // blocking read on pipe
        if(bytes_read > 0){
            printf("%c",c);
            //pthread_cond_signal(m_cv);
        }
    }while(bytes_read != 0);
   // pthread_cond_signal(m_cv);
    pthread_exit(NULL);
}

static void * control_thread(void * arg){
    while(1){
        printf("%s: threading pid: %i \n",__func__,pid);
        break;
    }
    pthread_exit(NULL);
}