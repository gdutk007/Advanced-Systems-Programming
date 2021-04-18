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
#include <ctype.h>
#include "simulator.h"

static int caps_history[4096];
static int caps_delim = 0;
static int int_endpoint[2];
static int ctrl_endpoint1[2];
static int ctrl_endpoint2[2];

static int text_file_fd;

static int send_control = 0;

void input_report_key(char c);

static void * interrupt_keyboard_thread(void * arg);
static void * control_keyboard_thread(void * arg);
static void * interrupt_driver_thread(void * arg);
static void * control_driver_thread(void * arg);
static void * usb_kbd_irq(void * arg);
static void * usb_kbd_event(void * arg);

pthread_mutex_t control_mutex;
pthread_cond_t  control_cv;

pthread_mutex_t * m_lock_int;
pthread_mutex_t * m_lock_led;
pthread_cond_t *  m_cv_int;

int * done_reading;
int * led_buff;

volatile int CAPS_LOCK = 0;

static int * kill_thread;
static int * kill_control;

int main(int argc, char *argv[]){

    pthread_mutexattr_t psharedm;
    pthread_mutexattr_t psharedm_led;
    pthread_condattr_t psharedc;
    pthread_mutexattr_init(&psharedm);
    pthread_mutexattr_init(&psharedm_led);
    pthread_mutexattr_setpshared(&psharedm,PTHREAD_PROCESS_SHARED);
    pthread_mutexattr_setpshared(&psharedm_led,PTHREAD_PROCESS_SHARED);
    pthread_condattr_init(&psharedc);
    pthread_condattr_setpshared(&psharedc,PTHREAD_PROCESS_SHARED);

    m_lock_int = (pthread_mutex_t * )mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    m_cv_int   = (pthread_cond_t * )mmap(NULL, sizeof(pthread_cond_t), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    m_lock_led = (pthread_mutex_t *)mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    done_reading = (int*) mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    led_buff = (int*) mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    kill_thread = (int*) mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    kill_control = (int*) mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *kill_control = 0;
    *done_reading = 1;
    *led_buff = 0;
    *kill_thread = 0;
    // init mutex and cond variable
    pthread_mutex_init(m_lock_int, &psharedm);
    pthread_mutex_init(m_lock_led, &psharedm_led);
    pthread_cond_init(m_cv_int, &psharedc);

    pthread_mutex_init(&control_mutex, NULL);
    pthread_cond_init(&control_cv,NULL);

    if(argc != 2){
        printf("Please enter a file name for the simulator to parse.\n");
        exit(0);
    }

    // try to open file, if we can't we must exit gracefully
    text_file_fd = open(argv[1], O_RDWR);
    if(text_file_fd < 0){
        printf("File could not be opened. Failed with: %s\n",strerror(errno));
        exit(0);
    }
    setvbuf(stdin,NULL,_IONBF,0);
    setvbuf(stdout,NULL,_IONBF,0);
    // create pipes
    pipe(int_endpoint);
    pipe(ctrl_endpoint1);
    pipe(ctrl_endpoint2);

    int status = 0;
    pid_t child_a, child_b;
    pthread_t mainT;
    int thread;
    child_a = fork();
    // child a will be the driver
    close(ctrl_endpoint1[0]);
    close(ctrl_endpoint1[1]);
    if(!child_a){
        // the keyboard (child a)
        pthread_t tid[2];
        close(int_endpoint[0]);
        fcntl(ctrl_endpoint2[0], F_SETFL, O_NONBLOCK);
        pthread_create(&tid[0], NULL, interrupt_keyboard_thread, NULL);
        pthread_create(&tid[1], NULL, control_keyboard_thread, NULL);
        pthread_join(tid[0], NULL);
        pthread_join(tid[1], NULL);
    }else{
        child_b = fork();
        if(!child_b){
            // USB driver sim child b 
            pthread_t tid[2];
            close(int_endpoint[1]);
            fcntl(int_endpoint[0], F_SETFL, O_NONBLOCK);
            pthread_create(&tid[0], NULL, interrupt_driver_thread, NULL);
            pthread_create(&tid[1], NULL, control_driver_thread, NULL);
            pthread_join(tid[0], NULL);
            pthread_join(tid[1], NULL);
        }
    }
    // close pipes
    close(int_endpoint[0]);
    close(int_endpoint[1]);
    // close(ctrl_endpoint1[0]);
    // close(ctrl_endpoint1[1]);
    close(ctrl_endpoint2[0]);
    close(ctrl_endpoint2[1]);
    // wait for processes
    waitpid(child_a,&status,0);
    waitpid(child_b,&status,0);
    caps_history[caps_delim] = *led_buff;
    printf("\n");
    for(int i = 0; i < caps_delim; ++i){
        if(caps_history[i]){
            printf("ON ");
        }else{
            printf("OFF ");
        }
    }
    printf("\n");
    return 0;
}

static void * interrupt_keyboard_thread(void * arg){
    char c = '\0';
    ssize_t bytes_read = 0;
    size_t i = 0;
    printf("%s: threading pid: %i \n",__func__,getpid());
    int signaled = 0;
    do{
        c = '\0';
        //printf("%s: about to get lock. pid:  %i \n",__func__,getpid());
        pthread_mutex_lock(m_lock_int);
        while(*done_reading == 0 ){
            //printf("%s: waiting...\n",__func__);
            pthread_cond_wait(m_cv_int,m_lock_int);
        }
        //printf("%s: got lock, about to read. pid:  %i \n",__func__,getpid());
        *done_reading = 0;
        pthread_mutex_lock(m_lock_led);
        bytes_read = read(text_file_fd,&c,1);
        if(bytes_read > 0){
            //printf("caps: %i pid: %i\n",CAPS_LOCK,getpid());
            if(isalpha(c) && CAPS_LOCK){
                if((c >96) && (c <123)) c ^=0x20;
            }
            if (write(int_endpoint[1], &c, bytes_read) != bytes_read)
                         printf("Failed writing with errno: %s \n",strerror(errno));
        }
        //printf("%s: got to unlock. pid:  %i \n",__func__,getpid());
        pthread_mutex_unlock(m_lock_led);
        pthread_mutex_unlock(m_lock_int);
        //sleep(3);
        //printf("%s: unlocked, about to wait. pid:  %i \n",__func__,getpid());
    }while(bytes_read != 0);

    pthread_mutex_lock(m_lock_int);
    *done_reading = -1;
    pthread_mutex_unlock(m_lock_int);

    *kill_thread = 1;
    printf("\n%s: exiting thread: %i \n",__func__,getpid());
    close(int_endpoint[1]);
    pthread_exit(NULL);
}

static void * control_keyboard_thread(void * arg){
    char c = '\0';
    ssize_t bytes_read = -1;
    size_t i = 0;
    while(*kill_control == 0){
        bytes_read = read(ctrl_endpoint2[0],&c,1); // blocking read on pipe
        if(bytes_read > 0){
            pthread_mutex_lock(m_lock_led);
            CAPS_LOCK = (*led_buff == 1) ? !CAPS_LOCK : CAPS_LOCK;  
            //printf("led buff: %i, caps after: %i pid: %i \n",*led_buff,CAPS_LOCK,getpid());
            //printf("letter %c, caps after: %i \n",c,CAPS_LOCK);
            pthread_mutex_unlock(m_lock_led);
        }
    }
    printf("%s: exiting thread: %i \n",__func__,getpid());
    pthread_exit(NULL);
}

static void * interrupt_driver_thread(void * arg){
    char c = '\0';
    ssize_t bytes_read = 0;
    size_t i = 0;
    int val = 0;
    printf("%s: threading pid: %i \n",__func__,getpid());
    do{
        c = '\0';
        //printf("%s: about to get lock. pid:  %i \n",__func__,getpid());
        pthread_mutex_lock(m_lock_int);
        //printf("%s: got lock, about to read. pid:  %i \n",__func__,getpid());
        bytes_read = read(int_endpoint[0],&c,1); // blocking read on pipe
        if(bytes_read > 0){
            // dispatch usb_kbd_irq
            pthread_t tid;
            char * read_c = malloc(sizeof(char));
            *read_c = c;
            pthread_create(&tid, NULL, usb_kbd_irq, read_c);
            pthread_join(tid, NULL);
            *done_reading = 1;
        }
        //printf("%s: got to unlock. pid:  %i \n",__func__,getpid());
        val = *done_reading;
        pthread_cond_signal(m_cv_int);
        pthread_mutex_unlock(m_lock_int);
        //printf("%s: unlocked, about to wait. pid:  %i \n",__func__,getpid());
    }while(val != -1);
    pthread_cond_signal(m_cv_int);
    *kill_thread = 1;
    pthread_mutex_lock(&control_mutex);
    send_control = 1;
    pthread_cond_signal(&control_cv);
    pthread_mutex_unlock(&control_mutex);
    printf("\n%s: exiting thread: %i \n",__func__,getpid());
    close(int_endpoint[0]);
    close(ctrl_endpoint2[1]);
    close(ctrl_endpoint2[0]);
    pthread_exit(NULL);
}

static void * control_driver_thread(void * arg){
    //printf("%s: exiting thread: %i \n",__func__,getpid());
    char c = 'C';
    while(*kill_thread == 0){
        pthread_mutex_lock(&control_mutex);
        while(!send_control) pthread_cond_wait(&control_cv,&control_mutex);
        if (write(ctrl_endpoint2[1], &c, 1) != 1)
                         printf("Failed writing with errno: %s \n",strerror(errno));
        send_control = 0;
        pthread_mutex_unlock(&control_mutex);
    }
    printf("\n%s: exiting thread: %i \n",__func__,getpid());
    *kill_control = 1;
    pthread_exit(NULL);
}

static void * usb_kbd_irq(void * arg){
    char c = *((char*)arg);
    free(arg);
    if(c != '#'){
        if(c == '@' || c == '&'){
            input_report_key(c);
        }else{
            printf("%c",c);
        }
    }
    //printf("%s: exiting thread: %i \n",__func__,getpid());
    pthread_exit(NULL);
}

void input_report_key(char c){
    pthread_t tid;
    char * read_c = malloc(sizeof(char));
    *read_c = c;
    pthread_create(&tid, NULL,usb_kbd_event,read_c);
    pthread_join(tid, NULL);
}

static void * usb_kbd_event(void * arg){
    char c = *((char*)arg);
    free(arg);
    pthread_mutex_lock(m_lock_led);
    *led_buff = !*led_buff;
    pthread_mutex_unlock(m_lock_led);

    pthread_mutex_lock(&control_mutex);
    send_control = 1;
    pthread_cond_signal(&control_cv);
    pthread_mutex_unlock(&control_mutex);

    //printf("  \n%s: exiting thread: %i \n",__func__,getpid());
    pthread_exit(NULL);
}