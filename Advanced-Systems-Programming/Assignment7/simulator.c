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

void readFile(char *argv[]);

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
    // read file
    readFile(argv);
    
    child_a = fork();
    if(!child_a){
        // setup params to pass pipe fds
        process_param_t process_params1;    
    }else{
        child_b = fork();
        if(!child_b){
            // setup params to pass pipe fds
            process_param_t process_params2;    
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