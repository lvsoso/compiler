#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

#define KB_DEVICE_FILE "/dev/input/event3"

int main(int argc, char *argv[])
{
    int fd = -1, ret = -1;
    struct input_event in;
    char *kbstatestr[] = {"keyUp", "keyDown"};
	char *kbsyn[] = {"start", "keyboard", "end"};

    // open device
    fd = open(KB_DEVICE_FILE, O_RDONLY);
	if (fd < 0)
	{
		perror("open device faild");
		return -1;
	}

while (1)
	{
		// read event
		ret = read(fd, &in, sizeof(struct input_event));
		if (ret != sizeof(struct input_event))
		{
			perror("read  device faild");
			break;
		}
		//parse event
		if (in.type == 1)
		{
			printf("------------------------------------\n");
			printf("STATUS:%s TYPE:%s CODE:%d TIME:%ld\n", kbstatestr[in.value], kbsyn[in.type], in.code, in.time.tv_usec);
			if (in.code == 46)
			{
				break;
			}
		}
	}

    // close
    close(fd);
    return 0;
}

