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

    /*           Case 1
   for case 1  the device driver 
   seems to be locking semaphore 2 and not 
   releasing it before exiting during open(). Opening 
   the driver several times will cause 
   the locked semaphore to deadlock. 

     -The first open will take the lock
     - The second open will get blocked. 
     - Since the second open is blocking, 
     the third open never gets called. The 
     threads stay waiting on the semaphore 
     that never gets released. 
    */

static void * mainThread(void * arg){
    int fd1, fd2, fd3;
    fd1 = fd2 = fd3 = 0;
    /*
        we will open several times since the open 
        file operation mistakenly locks before 
        exiting. 
    */
    fd1 = open("/dev/a5", O_RDWR);
    fd2 = open("/dev/a5", O_RDWR);
    fd3 = open("/dev/a5", O_RDWR);
    pthread_exit(nullptr);
}



int main(int argc, char *argv[]){
    // this will be main thread
    pthread_t mainT;
    int pThread;
    pThread = pthread_create(&mainT,nullptr,mainThread,(void*)argv);
    pThread = pthread_create(&mainT,nullptr,mainThread,(void*)argv);
    pthread_exit(nullptr);
    return 0;
}