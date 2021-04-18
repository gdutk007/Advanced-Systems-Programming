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
#define LED_BUFFER "/led_buffer"
#define LED_MUTEX  "/led_mutex"
#define INT_MUTEX  "/int_mutex"
#define COND_VAR   "/cond_var"

// keys buffer
static char buffer[MAX_BUFF];

static int int_endpoint[2];
static int ctrl_endpoint1[2];
static int ctrl_endpoint2[2];

static int led_buffer_fd; 
static int led_mutex_fd;
static int cond_var_fd;
static int int_mutex_fd;


void init_shared_mem();

int main(int argc, char *argv[]){
    if(argc != 2){
        printf("Please enter a file name for the simulator to parse.\n");
        exit(0);
    }
    setvbuf(stdin,NULL,_IONBF,0);
    setvbuf(stdout,NULL,_IONBF,0);

    init_shared_mem(); // init shared mem
    
    // try to open file, if we can't we must exit gracefully
    int fd = open(argv[1], O_RDWR);
    if(fd < 0){
        printf("File could not be opened. Failed with: %s\n",strerror(errno));
        exit(0);
    }
    // create pipes
    pipe(int_endpoint);
    pipe(ctrl_endpoint1);
    pipe(ctrl_endpoint2);
    // this will be main thread
    int status = 0;
    pid_t child_a, child_b;
    pthread_t mainT;
    int thread;    
    child_a = fork();
    // child a will be the driver
    if(!child_a){
        // setup params to pass pipe fds driver_simulator
        char * driver_executable   = "./driver_simulator";
        char fd1[4], fd2[4], fd3[4],fd4[4], fd5[4], fd6[4], fd7[4];
        snprintf(fd1, 10, "%d", int_endpoint[0]);   // read interrupt char
        snprintf(fd2, 10, "%d", ctrl_endpoint1[0]); // read ack
        snprintf(fd3, 10, "%d", ctrl_endpoint2[1]); // write control endpoint 
        snprintf(fd4, 10, "%d", led_buffer_fd);     // led buffer
        snprintf(fd5, 10, "%d", led_mutex_fd);      // led mutex
        snprintf(fd6, 10, "%d", cond_var_fd);       // cond var 
        snprintf(fd7, 10, "%d", int_mutex_fd);      // interrupt mutex 

        char *args[9];
        args[0] = driver_executable; // executable
        args[1] = fd1;               // interrupt endpoint
        args[2] = fd2;               // ack endpoint
        args[3] = fd3;               // control endpoint
        args[4] = fd4;               // led buffer
        args[5] = fd5;               // led mutex
        args[6] = fd6;               // cond var
        args[7] = fd7;               // interrupt mutex
        args[8] = NULL;              // end
        // exec driver
        execv(args[0], args);
        fprintf(stderr, "Failed to execute '%s'\n", args[0]);
    }else{
        child_b = fork();
        if(!child_b){
            // setup params to pass pipe fds to keyboard
            char * keyboard_executable = "./keyboard";
            char fd1[4], fd2[4], fd3[4], fd4[4],fd5[4],fd6[4],fd7[4], fd8[4];
            snprintf(fd1, 10, "%d", fd);                // file descriptor with out text
            snprintf(fd2, 10, "%d", int_endpoint[1]);   // write interrupt char
            snprintf(fd3, 10, "%d", ctrl_endpoint1[1]); // write ack
            snprintf(fd4, 10, "%d", ctrl_endpoint2[0]); // read control endpoint
            snprintf(fd5, 10, "%d", led_buffer_fd);     // led buffer
            snprintf(fd6, 10, "%d", led_mutex_fd);      // led mutex
            snprintf(fd7, 10, "%d", cond_var_fd);       // cond var 
            snprintf(fd8, 10, "%d", int_mutex_fd);      // interrupt mutex 
            
            char *args[9];
            args[0] = keyboard_executable; // executable
            args[1] = fd1;                 // test file fd
            args[2] = fd2;                 // int endpoint
            args[3] = fd3;                 // ack fd
            args[4] = fd4;                 // control endpoint
            args[5] = fd5;                 // led buffer
            args[6] = fd6;                 // led mutex
            args[7] = fd7;                 // cond var
            args[8] = fd8;                 // interrupt mutex
            args[9] = NULL;                // end
            // exec driver
            execv(args[0], args);
            fprintf(stderr, "Failed to execute '%s'\n", args[0]);
        }
    }
    // close pipes
    close(int_endpoint[0]);
    close(int_endpoint[1]);
    close(ctrl_endpoint1[0]);
    close(ctrl_endpoint1[1]);
    close(ctrl_endpoint2[0]);
    close(ctrl_endpoint2[1]);
    // wait for processes
    waitpid(child_a,&status,0);
    waitpid(child_b,&status,0);
    return 0;

}

void init_shared_mem(){
    // get shared memory file descriptor (NOT a file)
	// open("/tmp/msyncTest", (O_CREAT | O_TRUNC | O_RDWR), (S_IRWXU | S_IRWXG | S_IRWXO) ); // might be an alternative
    led_buffer_fd = shm_open(LED_BUFFER, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if(led_buffer_fd < 0){
        printf("File could not be opened. Failed with: %s\n",strerror(errno));
        exit(0);
    }else{
        if(ftruncate(led_buffer_fd,sizeof(char))){
            printf("Ftruncate failed. Failed with: %s\n",strerror(errno));
        }
    }

    led_mutex_fd = shm_open(LED_MUTEX, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if(led_mutex_fd < 0){
        printf("File could not be opened. Failed with: %s\n",strerror(errno));
        exit(0);
    }else{
        if( ftruncate(led_mutex_fd,sizeof(pthread_mutex_t)) ){
            printf("Ftruncate failed. Failed with: %s\n",strerror(errno));
        }
    }

    cond_var_fd = shm_open(COND_VAR, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if(cond_var_fd < 0){
        printf("File could not be opened. Failed with: %s\n",strerror(errno));
        exit(0);
    }else{
        if( ftruncate(cond_var_fd,sizeof(pthread_cond_t)) ){
            printf("Ftruncate failed. Failed with: %s\n",strerror(errno));
        }
    }

    int_mutex_fd = shm_open(INT_MUTEX, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if(cond_var_fd < 0){
        printf("File could not be opened. Failed with: %s\n",strerror(errno));
        exit(0);
    }else{
        if( ftruncate(int_mutex_fd,sizeof(pthread_mutex_t)) ){
            printf("Ftruncate failed. Failed with: %s\n",strerror(errno));
        }
    }

    char * addr =(char *)mmap(NULL, sizeof(char),PROT_READ | PROT_WRITE,MAP_SHARED, led_buffer_fd, 0);
    *buffer = '0'; // init to zero
    // init synchronization variables
    pthread_mutexattr_t psharedm;
    pthread_condattr_t psharedc;
    pthread_mutexattr_t psharedm_int;
    pthread_mutexattr_init(&psharedm_int);
    pthread_mutexattr_init(&psharedm);
    pthread_mutexattr_setpshared(&psharedm_int,PTHREAD_PROCESS_SHARED);
    pthread_mutexattr_setpshared(&psharedm,PTHREAD_PROCESS_SHARED);
    pthread_condattr_init(&psharedc);
    pthread_condattr_setpshared(&psharedc,PTHREAD_PROCESS_SHARED);
    // create shared mutex
    pthread_mutex_t * m_Lock = (pthread_mutex_t * ) mmap(NULL, sizeof(pthread_mutex_t),
                                                 PROT_READ | PROT_WRITE,MAP_SHARED, led_mutex_fd, 0);
    // create shared condition variable
    pthread_cond_t * m_cv = (pthread_cond_t * ) mmap(NULL, sizeof(pthread_cond_t), 
                                            PROT_READ | PROT_WRITE,MAP_SHARED, cond_var_fd, 0);
    pthread_mutex_t * m_int_Lock = (pthread_mutex_t * ) mmap(NULL, sizeof(pthread_mutex_t),
                                                 PROT_READ | PROT_WRITE,MAP_SHARED, int_mutex_fd, 0);
    // init mutex and cond variable
    pthread_mutex_init(m_Lock, &psharedm);
    pthread_mutex_init(m_int_Lock, &psharedm_int);
    pthread_cond_init(m_cv, &psharedc);
}