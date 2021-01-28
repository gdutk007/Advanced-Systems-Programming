#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>


static char buff[10240];

int main(){
int status = 0;
pid_t child_a, child_b;
int pfd[2];
pipe(pfd);
child_a = fork();

if (child_a == 0) {
    /* Child A code */
    if(close(pfd[0] == -1)) printf("ERROR");
    // duplicate stdout
    dup2(pfd[1],STDOUT_FILENO);
    close(pfd[1]);

    const char * email_filter = "./email_filter";
    execlp(email_filter,email_filter,(char*)NULL);
    fprintf(stderr, "Failed to execute '%s'\n", email_filter);
} else {
    child_b = fork();

    if (child_b == 0) {
        /* Child B code */
        if(close(pfd[1])) printf("ERROR");
        dup2(pfd[0],STDIN_FILENO);
        close(pfd[0]);
        const char * calendar_filter = "./calendar_filter";
        execlp(calendar_filter,calendar_filter,(char*)NULL);
        fprintf(stderr, "Failed to execute '%s'\n", calendar_filter);
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