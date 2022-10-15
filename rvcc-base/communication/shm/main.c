#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <unistd.h>
#include <errno.h>

static int shmid;


#define SHM_TYPE (041375)
#define SHM_KEY (752364)
#define SHM_BODY_SIZE (4096-8)
#define SHM_STATUS (SHM_BODY_SIZE)

typedef struct SHM
{
    long status;
    char body[SHM_BODY_SIZE];
} shm_t;


int receive_main(int mid)
{
	// 绑定共享内存
	int ok = 0;
	shm_t* addr = shmat(mid, NULL, 0);
	if ((long)addr < 0)
	{
		perror("binding share memory faild\n");
	}
	printf("child process access share memory address : %p\n", addr);

	while (1)
	{
		if(addr->status == SHM_STATUS)
		{
			for (int i = 0; i < SHM_BODY_SIZE; i++)
			{
				if (addr->body[i] != (char)0xff)
				{
					printf("check share data faild:%x\n", addr->body[i]);
				}
				else
				{
					ok++;
				}
				
			}
			printf("check share data success:%d\n", ok);

			return 0;
		}
		sleep(2);
	}
	return 0;
}
int send_main(int mid)
{
    // bind share memory
    shm_t* addr = shmat(mid, NULL, 0);
    if((long)addr <0)
    {
        perror("binding share memory faild\n");
    }

    printf("main process access share memory address : %p\n", addr);
    memset(addr, 0xff, sizeof(shm_t));
    
    // sync status
    addr->status = SHM_STATUS;
    
    // wait child process exit
    wait(NULL);
	return 0;
}

int main()
{
    pid_t pid;

    // create share memory;
    shmid = shmget(SHM_KEY, sizeof(shm_t), IPC_CREAT| 0666);
    if(shmid <0)
    {
        perror("create share memory failed\n");
    }

    // create child process;
    pid = fork();
    if(pid>-0)
    {
        // main process
        send_main(shmid);
    }
	else if (pid == 0)
	{
		// new child process
		receive_main(shmid);
	}
	return 0;
}