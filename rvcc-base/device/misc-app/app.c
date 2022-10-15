
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define DEV_NAME "/dev/miscdevtest"

int main(void)
{
    char buf[] = {0, 0, 0, 0};
    int i = 0;
    int fd;

    //open device O_RDWR,  O_RDONLY, O_WRONLY,
    fd = open(DEV_NAME, O_RDWR);
    if (fd < 0)
    {
        printf("打开 :%s 失败!\n", DEV_NAME);
    }
    //write data to kenerl space
    write(fd, buf, 4);

    //read data from kenerl space
    read(fd, buf, 4);

    //close device
    close(fd);

    return 0;
}