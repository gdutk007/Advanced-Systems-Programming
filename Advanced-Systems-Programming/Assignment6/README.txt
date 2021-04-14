README

1. RUNNING AND COMPILING THE CODE

 To make the Driver you should only need to run the 
 following command: 

    "make"

 In order to build the deadlock cases, please run the following 
 code: 

     "make app"

This should create the kernel driver and the programs which 
produce deadlock scenerios. To run the deadlock scenarios 
you need to make sure to run them with sudo privileges. 







2. DEADLOCK SCENERIOS: 

/*           CASE 1
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

/*           CASE 2
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

/*           CASE 3
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

/*           CASE 3
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








3.          POSSIBLE RACE CONDITIONS
/*                  CASE 1 
    The first Critical Section with a race condition is 
    the Write() and Read() with the Character Device Driver set to MODE2.
    
    When multiple threads are writing and reading to the ramdisk buffer we don't 
    see the driver using the file offset. This can cause multiple threads 
    writing to the buffer to overwrite data written by the 
    previous thread. This would mean that when reading from the 
    ramdisk buffer data will be read incorrectly. The specific line is 
    the following: 
        copy_from_user(devc->ramdisk, buf, count);   (Line 127)
        and 
        ret = count - copy_to_user(buf, devc->ramdisk, count); (Line 100)
    
    While the ramdisk buffer is protected by semaphore 1, it is still a race condition 
    since we are not keeping track of the position and we cannot guarantee the data.
    When reading and semaphore 1 is locked before reading/writing and unlocked after 
    reading/writing. 
*/

/*                  CASE 2
    Another region of code that has a race condition is the read and write operations 
    during "MODE 1". During "MODE 1" read and write does not protect the ramdisk buffer.
    This means that if someone tried to run several read and write threads in parallel 
    the data in the buffer could be overwritten and misread. This is similar to Case 1 
    but even more incorrect since sempahore 1 is released before reading and writing. 
*/

/*                  CASE 3
    The next pair of critical sections that has race conditions is the critical section 
    during open() and during the ioctl() functions when we are switching back to "MODE 1".

    If the driver is in MODE 2 and multiple threads are calling open() the driver will increment the 
    mode 2 counter by 1 each time. This is problematic if any of the threads are trying to switch 
    back to mode  1. ioctl() would be called to switch back to mode 1 and the driver will block 
    until the mode 2 counter is decremented to 1. To avoid deadlocks and for the number to be correct, 
    it would be better if the driver would decrement the count befor waiting. That way if every thread 
    tried to switch to MODE 1, it would update the value immediately. The mode 2 counter would reflect 
    the real number of threads trying to be in mode 1.  

*/

/*                  CASE 4
    The last pair of critical sections I will cover are the critical sections of open()
    and the release() functions. These two critical sections do not have race conditions.
    They make sure to aquire the semaphore 1 lock before doing anything to count1 and 
    count2. The prevents buggy behavior reading and writing to these values. After they 
    are done they release their locks and the functions exit.  
*/

