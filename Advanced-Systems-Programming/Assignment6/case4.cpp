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
        For this third case two threads will try and switch their modes. 

        While trying to switch modes there can be a case where both 
        threads are left waiting for locks 1 and 2 to be released. This 
        will never happen under the following scenerio. 

           - The first thread will open the driver and switch its mode to mode 2 and sleep. 
           - The second thread will then open the driver and lock semaphore 2.
           - Then thread 2 will try to switch to mode 1, but will block trying to lock 
           sempahore 2 because open() already locked it. 
           - Now because we block trying to switch back to mode 1, we will be back in thread
           1. Thread 1 will try to switch the mode to mode 1 and will fail because thread 2 
           has locked sempahores 1 and 2 when trying to switch its mode. 
    */

static void * thread1(void * arg){
    int fd1;
    int io;
    fd1 = 0;
    io = 0;
    fd1 = open("/dev/a5", O_RDWR);
    cout << "Changing mode in thread...\n";
    io = ioctl(fd1,E2_IOCMODE1,0);            
    int ret = close(fd1);
    pthread_exit(nullptr);
}


int main(int argc, char *argv[]){
    // this will be main thread
    pthread_t mainT;
    int pThread;
    
    int fd = 0;
    int io = 0;
    fd = open("/dev/a5", O_RDWR);
    io = ioctl(fd,E2_IOCMODE2,0);            


    pThread = pthread_create(&mainT,nullptr,thread1,(void*)argv);
    pThread = pthread_create(&mainT,nullptr,thread1,(void*)argv);
    pThread = pthread_create(&mainT,nullptr,thread1,(void*)argv);
    pThread = pthread_create(&mainT,nullptr,thread1,(void*)argv);
    pThread = pthread_create(&mainT,nullptr,thread1,(void*)argv);
    pthread_exit(nullptr);
    return 0;
}