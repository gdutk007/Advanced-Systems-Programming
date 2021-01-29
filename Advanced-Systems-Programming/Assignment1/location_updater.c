#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>


static char buff[100];

int main(){
setvbuf(stdin,NULL,_IONBF,0);
setvbuf(stdout,NULL,_IONBF,0);

int status = 0;
pid_t child_a, child_b;
int pfd[2];
pipe(pfd);
child_a = fork();

if (child_a == 0) {
    /* Child A code */
	setvbuf(stdin,NULL,_IONBF,0);
	setvbuf(stdout,NULL,_IONBF,0);
    close(pfd[0]);
    dup2(pfd[1],STDOUT_FILENO);
    close(pfd[1]);
    // const char* msg1 = "hello 1";
    // const char* msg2 = "hello 2";
    // const char* msg3 = "hello 3";
    // int MSGSIZE = 7; 
    // write(pfd[1], msg1, MSGSIZE); 
    // write(pfd[1], msg2, MSGSIZE); 
    // write(pfd[1], msg3, MSGSIZE); 

    const char * email_filter = "./email_filter";
    execlp(email_filter,email_filter,(char*)NULL);
    fprintf(stderr, "Failed to execute '%s'\n", email_filter);
} else {
	setvbuf(stdin,NULL,_IONBF,0);
	setvbuf(stdout,NULL,_IONBF,0);
    child_b = fork();

    if (child_b == 0) {
        /* Child B code */
        setvbuf(stdin,NULL,_IONBF,0);
        setvbuf(stdout,NULL,_IONBF,0);
        close(pfd[1]);
        dup2(pfd[0],STDIN_FILENO);
        close(pfd[0]);
        // __ssize_t numRead = 0;
        // close(pfd[1]);
        // for(;;){
        //     numRead = read(pfd[0],buff,100);
        //     if (numRead == -1)
        //         printf("read failed...\n");
        //     if (numRead == 0)
        //         break;                      /* End-of-file */
        //     if(write(STDOUT_FILENO, buff, numRead) != numRead)
        //     {
        //         printf("child - partial/failed write");
        //         exit(1);
        //     }        
        // }
        printf(" INSIDE SECOND THREAD\n");
        const char * calendar_filter = "./calendar_filter";//"./calendar_filter";
        execlp(calendar_filter,calendar_filter,(char*)NULL);
        //execlp("wc","wc",(char*)NULL);
        //fprintf(stderr, "Failed to execute '%s'\n", calendar_filter);
        //fprintf(stderr, "Failed to execute '%s'\n", calendar_filter);
        //sleep(3);
    } else {
        /* Parent Code */
    }
}
    close(pfd[0]);
    close(pfd[1]);
    waitpid(child_a,&status,0);
    waitpid(child_b,&status,0);
    return 0;
}