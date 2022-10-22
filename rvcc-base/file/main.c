#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int main()
{
    
    struct stat filestat;
	int fd = -1;
	
    char ch[] = {0, 1, 0xff, 'L', 'V', 'S', 'O', 'S','O'};
	
    // 打开并建立文件,所有用户可读写
	fd = open("empty.file", O_RDWR|O_CREAT, S_IRWXU|S_IRWXO|S_IRWXG);
	if(fd < 0)
	{
		printf("建立文件失败\n");
		return -1;
	}

	write(fd, ch, 9);
	// 获取文件信息，比如文件大小
	fstat(fd, &filestat);
	printf("文件大小:%ld\n", filestat.st_size);
	printf("文件模式:%d\n", filestat.st_mode);
	printf("文件节点号:%ld\n", filestat.st_ino);
	printf("文件所在设备号:%ld\n", filestat.st_dev);
	printf("文件特殊设备号:%ld\n", filestat.st_rdev);
	printf("文件连接数:%ld\n", filestat.st_nlink);
	printf("文件所属用户:%d\n", filestat.st_uid);
	printf("文件所属用户组:%d\n", filestat.st_gid);
	printf("文件最后访问时间:%ld\n", filestat.st_atime);
	printf("文件最后修改时间:%ld\n", filestat.st_mtime);
	printf("文件状态改娈时间:%ld\n", filestat.st_ctime);
	printf("文件对应的块大小:%ld\n", filestat.st_blksize);
	printf("文件占用多少块:%ld\n", filestat.st_blocks);
	// 关闭文件
	close(fd);
	return 0;
}