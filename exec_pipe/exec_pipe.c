#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]){
    if(argc <= 1){
        fprintf(stderr, "Usage: %s <target>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    pid_t pid;

    int errpipe[2];
    if(pipe(errpipe) != 0){
        fprintf(stderr, "Error: pipe\n");
        exit(EXIT_FAILURE);
    }

    if((pid = fork()) == -1){
        fprintf(stderr, "fork error\n");
        exit(EXIT_FAILURE);
    }
    else if(pid == 0){ //child
        close(errpipe[0]);
        dup2(errpipe[1], STDOUT_FILENO);
        close(errpipe[1]);
        execv(argv[1], NULL);
    }
    else{ //parent
        close(errpipe[1]);
        int status;
        waitpid(pid, &status, 0);
        char buf[256];
        read(errpipe[0], buf, 256);
        printf("result:%s\n", buf);
        close(errpipe[0]);
    }

    return 0;
}