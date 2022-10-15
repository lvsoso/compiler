#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <errno.h>
static pid_t subid;
static int msgid;

#define MSG_TYPE (041375)
#define MSG_KEY (752364)
#define MSG_SIZE (256)
typedef struct Msg
{
	long type;
	char body[MSG_SIZE];
} msg_t;

int receive_main(int mid)
{
	msg_t msg;
	while (1)
	{
		ssize_t sz = msgrcv(mid, &msg, MSG_SIZE, MSG_TYPE, MSG_NOERROR);
		if (sz < 0)
		{
			perror("get msg faild");
		}
		printf("new message:%s\n", msg.body);
		//check if exit
		if (strncmp("exit", msg.body, 4) == 0)
		{
			printf("end\n");
			exit(0);
		}
	}
	return 0;
}

int send_main(int mid)
{
	msg_t msg;

	while (1)
	{
		// set msg type
		msg.type = MSG_TYPE;
        // get from stdin
		scanf("%[^\n]%*c", msg.body);
		// send msg
		msgsnd(mid, &msg, MSG_SIZE, 0);
		// check if exit
		if (strncmp("exit", msg.body, 4) == 0)
		{
			return 0;
		}
	}
	return 0;
}

int main()
{
	pid_t pid;
	msgid = msgget(MSG_KEY, IPC_CREAT | 0666);
	if (msgid < 0)
	{
		perror("create msg queue faild\n");
	}

	pid = fork();
	if (pid > 0)
	{
		subid = pid;
		send_main(msgid);
	}
	else if (pid == 0)
	{
		receive_main(msgid);
	}
	return 0;
}
