#include <iostream>
#include <fstream>
#include <bits/stdc++.h>
#include <cstdlib>
#include <string>
#include <semaphore.h>
#include <pthread.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

using namespace std;

#define CDRV_IOC_MAGIC 'Z'
#define E2_IOCMODE1 _IOWR(CDRV_IOC_MAGIC, 1, int)
#define E2_IOCMODE2 _IOWR(CDRV_IOC_MAGIC, 2, int)


    /*           Case 3
        
    */

static void * thread1(void * arg){
    int fd1,fd2;
    int io;
    fd1 = 0;
    io = 0;

    fd1 = open("/dev/a5", O_RDWR);
    io = ioctl(fd1,E2_IOCMODE2,0);            
    sleep(4);
    fd2 = open("/dev/a5", O_RDWR); 
    pthread_exit(nullptr);
}

static void * thread2(void * arg){
    int fd1,fd2;
    int io;
    fd1 = 0;
    io = 0;
    sleep(1); // we want to make sure this doesn't run first
    fd1 = open("/dev/a5", O_RDWR);
    io = ioctl(fd1,E2_IOCMODE1,0); // we should block switching to mode 1 
    pthread_exit(nullptr);
}

int main(int argc, char *argv[]){
    // this will be main thread
    pthread_t mainT;
    int pThread;
    pThread = pthread_create(&mainT,nullptr,thread1,(void*)argv);
    pThread = pthread_create(&mainT,nullptr,thread2,(void*)argv);
    pthread_exit(nullptr);
    return 0;
}