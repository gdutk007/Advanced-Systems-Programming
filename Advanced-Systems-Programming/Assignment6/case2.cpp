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


    /*           Case 2
        for case 2 we will have two threads and the deadlock happens 
        when switching from mode 1 to mode 2. 
        - First we open the driver twice, this will block 
        on the second open. 
        - When we block on the second open, we switch back 
        to thread 1 and try to use ioctl to switch to mode 2. 
        - When we try to switch to mode 2 we have to wait until 
        (devc->count1 == 1) so we start to block again. 
        - While blocking and waiting for (devc->count1 == 1) we switch 
        back to the other thread which is still blocked waiting for 
        semaphore 2 to become unlocked.  
    */

static void * thread1(void * arg){
    int fd1;
    int io;
    fd1 = 0;
    io = 0;

    fd1 = open("/dev/a5", O_RDWR);
    sleep(1);
    io = ioctl(fd1,E2_IOCMODE2,0);            
    pthread_exit(nullptr);
}

int main(int argc, char *argv[]){
    // this will be main thread
    pthread_t mainT;
    int pThread;
    pThread = pthread_create(&mainT,nullptr,thread1,(void*)argv);
    pThread = pthread_create(&mainT,nullptr,thread1,(void*)argv);
    pthread_exit(nullptr);
    return 0;
}