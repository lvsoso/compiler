#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


int run(char*  cmd)
{
    pid_t pid;
    int rets;

    // create child process
    pid = fork();
    if(pid > 0)
    {
        wait(&rets);
    }
    else if(pid == 0)
    {
        if(execl(cmd, cmd, NULL) == -1)
        {
            printf("未找到该应用\n");
			exit(0);
        }
    }
    return 0;
}

int shell_run()
{
    char instr[80];
    while (1)
    {
        printf("请输出应用名称：");
        scanf("%[^\n]%*c", instr);
        if(strncmp("exit", instr, 4) == 0)
        {
            return 0;
        }
        run(instr);
    }
    return 0;
}

int main()
{
    return shell_run();
}