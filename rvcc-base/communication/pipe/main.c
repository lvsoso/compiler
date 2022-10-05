#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main()
{
    pid_t pid;
    int rets;
    int fd[2];

    char r_buf[1024] = {0};
    char w_buf[1024] = {0};

    sprintf(w_buf, "parent process id = %d \n", getpid());

    if (pipe(fd) < 0)
    {
        perror("create pipe failed\n");
    }

    pid = fork();
    if(pid > 0)
    {
        write(fd[1], w_buf, strlen(w_buf));
        wait(&rets);
    }
    else if (pid == 0)
    {
        printf("child process  id = %d\n", getpid());
    read(fd[0], r_buf, strlen(w_buf));
    printf("pipe output: %s\n", r_buf);
    }
}