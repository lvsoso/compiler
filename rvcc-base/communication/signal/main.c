#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static pid_t subid;

void handle_sigusr1(int signum, siginfo_t * info, void * ucontext)
{
    printf("handle)sigusr1 signal code: %d\n", signum);

    if(ucontext != NULL)
    {
        printf("child id in message :  %d\n", info->si_int);
        printf("parent id in message : %d\n", info->si_pid);

        printf("compare the recived child id and real child id:%d == Getpid:%d\n", info->si_value.sival_int, getpid());
    }
    exit(0);
    return;
}

int subprocmain()
{
	struct sigaction sig;

    sig.sa_sigaction = handle_sigusr1;
	sig.sa_flags = SA_SIGINFO;

    sigaction(SIGUSR1, &sig, NULL);
	while (1)
	{
		pause(); 
	}
	return 0;


}

void handle_timer(int signum, siginfo_t *info, void *ucontext)
{
	printf("handle_timer 信号码:%d\n", signum);
	union sigval value;
	value.sival_int = subid; 
	sigqueue(value.sival_int, SIGUSR1, value);
	return;
}

int main()
{
	pid_t pid;

	pid = fork();
	if (pid > 0)
	{

		subid = pid;
		struct sigaction sig;

		sig.sa_sigaction = handle_timer;
		sig.sa_flags = SA_SIGINFO;
		
		sigaction(SIGALRM, &sig, NULL);
		alarm(4);
		while (1)
		{
			pause();
		}
	}
	else if (pid == 0)
	{
		subprocmain();
	}
	return 0;
}