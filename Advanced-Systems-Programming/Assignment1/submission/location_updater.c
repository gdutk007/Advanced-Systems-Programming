#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>


int main(){

setvbuf(stdin,NULL,_IONBF,0);
setvbuf(stdout,NULL,_IONBF,0);

int status = 0;
pid_t child_a, child_b;
int pfd[2];

// create pipe
pipe(pfd);
child_a = fork();

if (child_a == 0) {
    /* Child A code */
	setvbuf(stdin,NULL,_IONBF,0);
	setvbuf(stdout,NULL,_IONBF,0);
    close(pfd[0]); // send process stdout to pipe
    dup2(pfd[1],STDOUT_FILENO);
    close(pfd[1]);
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
        dup2(pfd[0],STDIN_FILENO); // set pipe read end to stdin
        close(pfd[0]);
        const char * calendar_filter = "./calendar_filter";//"./calendar_filter";
        execlp(calendar_filter,calendar_filter,(char*)NULL);
        fprintf(stderr, "Failed to execute '%s'\n", calendar_filter);
    } else {
        /* Parent Code */
        // doesn't really do anything
    }
}
close(pfd[0]);
close(pfd[1]);
waitpid(child_a,&status,0);
waitpid(child_b,&status,0);
return 0;
}