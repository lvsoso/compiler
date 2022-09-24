#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

void waitforKeyc()
{
    char c;
	while(1)
	{
		printf("请输入c退出应用:");
		c = (char)getchar();
		if(c == 'c')
		{
			return;
		}
	}
	return;
}

void testcow()
{
    pid_t pid;
	printf("当前应用id = %d\n",getpid());
	pid = fork();
    if (pid > 0 ) {
		printf("这是当前应用,当前应用id = %d 新建应用id = %d\n", getpid(), pid);
    }else if (pid == 0){
		printf("这是新建应用,新建应用id = %d\n", getpid());
    }
    return;
}

void testrnp()
{
	size_t msize = 0x1000 * 1024;
	void* buf = NULL;
	printf("当前应用id = %d\n",getpid());
	buf = malloc(msize);
	if(buf == NULL)
	{
		printf("分配内存空间失败\n");
		return;
	}
	memset(buf, 0xaf, msize);
	printf("分配内存空间地址:%p 大小:%ld\n", buf, msize);
	return;
}

int main()
{
    // testcow();
    // waitforKeyc();

    testrnp();
    waitforKeyc();

	// size_t len = 0x1000;
	// void* buf = NULL;
	// int fd = -1;
	// printf("当前应用id = %d\n",getpid());
	// fd = open("./testmmap.bin", O_RDWR|O_CREAT|O_TRUNC, S_IRWXU|S_IRWXG|S_IRWXO);
	// if(fd < 0)
	// {
	// 	printf("打开文件失败\n");
	// 	return 0;
	// }
	// //因为mmap不能扩展空文件，空文件没有物理内存页，所以先要改变文件大小，否则产生总线错误
	// ftruncate(fd, len);
	// buf = mmap(NULL, len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	// if(buf == NULL) 
	// {
	// 	printf("映射文件失败\n");
	// 	return 0;
	// }
	// printf("映射文件的内存地址:%p 大小:%ld\n", buf, len);
	// //向文件映射区间写入0xff
	// memset(buf, 0xff, len);
	// close(fd);
	// //防止程序退出
	// waitforKeyc();
	return 0;
}