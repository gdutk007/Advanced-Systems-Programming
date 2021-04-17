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
#include "simulator.h"

#define MAX_BUFF 2048

// keys buffer
static char buffer[MAX_BUFF];

static int int_endpoint[2];
static int ctrl_endpoint1[2];
static int ctrl_endpoint2[2];

// process one
static void * InterruptThread(void * arg);
static void * ControlThread(void * arg);
//void readFile(char *argv[]);


int main(int argc, char *argv[]){

    if(argc != 2){
        printf("Please enter a file name for the simulator to parse.\n");
        exit(0);
    }
    setvbuf(stdin,NULL,_IONBF,0);
    setvbuf(stdout,NULL,_IONBF,0);
    // create pipes
    pipe(int_endpoint);
    pipe(ctrl_endpoint1);
    pipe(ctrl_endpoint2);
    // this will be main thread
    int status = 0;
    pid_t child_a, child_b;
    pthread_t mainT;
    int thread;

    // reading file
    //readFile(argv);
    thread = pthread_create(&mainT,NULL,InterruptThread,(void*)argv);
    thread = pthread_create(&mainT,NULL,ControlThread,(void*)argv);

    pthread_exit(NULL);
    return 0;
}

    /*int thread*/
static void * InterruptThread(void * arg){
    while(1){

    }
    pthread_exit(NULL);
}
    /*control*/
static void * ControlThread(void * arg){
    while(1){

    }
    pthread_exit(NULL);
}