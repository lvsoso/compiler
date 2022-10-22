#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "ext3fs.h"

#define SECTOR_SIZE (512)

int hdfilefd = -1;
void* hdaddr = NULL;
size_t hdsize = 0;
__u64 block_size = 123;

struct ext3_super_block* super = NULL;
struct ext3_inode* rootinode = NULL;
struct ext3_dir_entry* rootdir = NULL;
// 将扇区号转换成文件映射的虚拟内存地址
void* sector_to_addr(__u64 nr)
{
	return (void*)((__u64)hdaddr + (nr * SECTOR_SIZE));
}

// 将储存块号转换成文件映射的虚拟内存地址
void* block_to_addr(__u64 nr)
{
	return (void*)((__u64)hdaddr + (nr * block_size));
}

// 获取第一块组中的某个inode的地址
struct ext3_inode* get_inode(__u32 nr)
{
	__u32 gi = (nr - 1)/super->s_inodes_per_group;
	// 获取第1个块组描述符
	struct ext3_group_desc* group = (struct ext3_group_desc* ) block_to_addr(2);
	// 获取该块组的inode表的块号
	__u32 ino = group[gi].bg_inode_table;
	// 获取第nr个inode
	struct ext3_inode* inp = (struct ext3_inode* )((__u64)block_to_addr(ino)+(super->s_inode_size * ((nr - 1) - gi * super->s_inodes_per_group)));
	return inp;
}

// 获取根目录的inode的地址
struct ext3_inode* get_rootinode()
{
	// 获取第1个块组描述符
	struct ext3_group_desc* group = (struct ext3_group_desc* ) block_to_addr(2);
	// 获取该块组的inode表的块号
	__u32 ino = group->bg_inode_table;
	// 获取第二个inode
	struct ext3_inode* inp = (struct ext3_inode* )((__u64)block_to_addr(ino)+(super->s_inode_size*1));
	return inp;
}
// 获取根目录的开始的数据项的地址
struct ext3_dir_entry* get_rootdir()
{
	// 获取根目录的inode
	struct ext3_inode* inp = get_rootinode();
	// 返回根目录的inode中第一个数据块地址，就是根目录的数据
	return (struct ext3_dir_entry*)block_to_addr(inp->i_block[0]); 
}
// 获取超级块的地址
struct ext3_super_block* get_superblock()
{
	return (struct ext3_super_block*)sector_to_addr(2);
}

int init_in_hdfile()
{
	struct stat filestat;
	size_t len = 0;
	void* buf = NULL;
	int fd = -1;
	// 打开虚拟硬盘文件
	fd = open("./hd.img", O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO);
	if(fd < 0)
	{
		printf("打开文件失败\n");
		return -1;
	}
	// 获取文件信息，比如文件大小
	fstat(fd, &filestat);
	// 获取文件大小
	len = filestat.st_size;
	// 映射整个文件到进程的虚拟内存中
	buf = mmap(NULL, len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if(buf == NULL) 
	{
		printf("映射文件失败\n");
		return -2;
	}
	// 保存地址 长度大小 文件句柄 到全局变量
	hdaddr = buf;
	hdsize = len;
	hdfilefd = fd;
	super = get_superblock();
	block_size = (1 << super->s_log_block_size)*1024;

	rootinode = get_rootinode();
	rootdir = get_rootdir();
	return 0;
}

void dump_one_group(int i, struct ext3_group_desc* group)
{
	printf("第(%d)个块组位图块起始块号:%d\n", i, group->bg_block_bitmap);
	printf("第(%d)个块组inode节点位图起始块号:%d\n", i, group->bg_inode_bitmap);
	printf("第(%d)个块组inode节点表起始块号:%d\n", i, group->bg_inode_table);
	printf("第(%d)个块组空闲储存块:%d\n", i, group->bg_free_blocks_count);
	printf("第(%d)个块组空闲inode节点数:%d\n", i, group->bg_free_inodes_count);
	printf("第(%d)个块组目录计数:%d\n", i, group->bg_used_dirs_count);
	return;
}

void get_group_table(struct ext3_group_desc** outgtable, int* outnr )
{
	// 计算总块组数
	int gnr = super->s_blocks_count / super->s_blocks_per_group;
	// 获取块组描述表的首地址
	struct ext3_group_desc* group = (struct ext3_group_desc* ) block_to_addr(2);
	*outgtable = group;
	*outnr = gnr;
	return;
}

void dump_all_group()
{
	int gnr;
	struct ext3_group_desc* group;
	get_group_table(&group, &gnr);
	for(int i = 0; i < gnr; i++)
	{
		dump_one_group(i, &group[i]);
	}
	return;
}

void dump_inode(struct ext3_inode* inode)
{
	printf("文件权限模式:%d\n",inode->i_mode);
	printf("文件所属用户:%d\n",inode->i_uid);
	printf("文件所属用户组:%d\n",inode->i_gid);
	printf("文件大小:%d\n",inode->i_size);
	printf("文件占用的储存块数:%d\n",inode->i_blocks);
	printf("文件的链接数:%d\n",inode->i_links_count);
	printf("文件访问时间(秒):%d\n",inode->i_atime);
	printf("文件建立时间(秒):%d\n",inode->i_ctime);
	printf("文件修改时间(秒):%d\n",inode->i_mtime);
	for (int i = 0; i < EXT3_N_BLOCKS; i++)
	{
		printf("文件第%d个储存块号:%d\n", i, inode->i_block[i]);
	}
	
	return;
}

void dump_inode_data(struct ext3_inode* inode)
{
	// 获取文件大小
	__s64 filesize = inode->i_size;
	printf("----------------------------------------\n");
	// 展示文件inode的元信息
	dump_inode(inode);
	printf("----------------------------------------\n");
	for (int i = 0; (i < (EXT3_N_BLOCKS - 3))&&(filesize > 0); i++, filesize -= (__s64)block_size)
	{
		// 读取并打印每个储存块中数据内部
		printf("%s\n", (char*)block_to_addr(inode->i_block[i]));
	}
	return;
}

char* ret_file_type_str(__u8 type)
{
	if(type == 1)
	{
		return "文件";
	}
	if(type == 2)
	{
		return "目录";
	}
	if(type == 7)
	{
		return "符号链接";
	}
	return "未知";
}

struct ext3_dir_entry* get_next_dir_addr(struct ext3_dir_entry* dir)
{
	return (struct ext3_dir_entry*)((__u64)dir + dir->rec_len);
}

void dump_one_dir(struct ext3_dir_entry* dir)
{
	printf("目录项对应的inode节点:%d\n", dir->inode);
	printf("目录项长度:%d\n", dir->rec_len);
	printf("目录项名称长度:%d\n", dir->name_len);
	printf("目录项类型:%d, %s\n", dir->file_type, ret_file_type_str(dir->file_type));
	printf("目录项名称:%s\n", dir->name);
	return;
}

void dump_dirs_block(void* blk, size_t size)
{
	void* end = (void*)((__u64)blk + size);
	for (void* dir = blk; dir < end;)
	{
		dump_one_dir((struct ext3_dir_entry*)dir);
		dir = get_next_dir_addr((struct ext3_dir_entry*)dir); 
	}
	return;
}

void dump_dirs(struct ext3_inode* inode)
{
	__s64 filesize = inode->i_size;
	for (int i = 0; (i < (EXT3_N_BLOCKS - 3))&&(filesize > 0); i++, filesize -= (__s64)block_size)
	{
		dump_dirs_block(block_to_addr(inode->i_block[i]), (size_t)filesize);
	}
	return;
}


// 判定文件和目录
struct ext3_dir_entry* dir_file_is_ok(struct ext3_dir_entry* dire, __u8 type, char* name)
{
	// 比较文件和目录类型和名称
	if(dire->file_type == type)
	{
		if(0 == strncmp(name, dire->name, dire->name_len))
		{
			return dire;
		}
	}
	return NULL;
}
// 查找一个块中的目录项
struct ext3_dir_entry* find_dirs_on_block(void* blk, size_t size, __u8 type, char* name)
{
	struct ext3_dir_entry* dire = NULL;
	void* end = (void*)((__u64)blk + size);
	for (void* dir = blk; dir < end;)
	{
		// 判定是否找到
		dire = dir_file_is_ok((struct ext3_dir_entry*)dir, type, name);
		if(dire != NULL)
		{
			return dire;
		}
		// 获取下一个目录项地址
		dir = get_next_dir_addr((struct ext3_dir_entry*)dir); 
	}
	return NULL;
}
// 在一个目录文件中查找目录或者文件
struct ext3_dir_entry* find_dirs(struct ext3_inode* inode,  __u8 type, char* name)
{
	struct ext3_dir_entry* dir = NULL;
	__s64 filesize = inode->i_size;
	// 查找每个直接块
	for (int i = 0; (i < (EXT3_N_BLOCKS - 3))&&(filesize > 0); i++, filesize -= (__s64)block_size)
	{
		// 查找一个储存块
		dir = find_dirs_on_block(block_to_addr(inode->i_block[i]), (size_t)filesize, type, name);
		if(dir != NULL)
		{
			return dir;
		}
	}
	return NULL;
}


void dump_super_block(struct ext3_super_block* superblock)
{
	printf("inode节点总数:%d\n", superblock->s_inodes_count);
	printf("储存块总数:%d\n", superblock->s_blocks_count);
	printf("空闲inode节点总数:%d\n", superblock->s_free_inodes_count);
	printf("空闲储存块总数:%d\n", superblock->s_free_blocks_count);
	printf("第一个inode节点号:%d\n", superblock->s_first_ino);
	printf("第一个储存块:%d\n", superblock->s_first_data_block);
	printf("储存块大小:%d\n", (1 << superblock->s_log_block_size) * 1024);
	printf("每组包含inode节点数:%d\n", superblock->s_inodes_per_group);
	printf("每组包含储存块数:%d\n", superblock->s_blocks_per_group);
	printf("最后写入时间:%d秒\n", superblock->s_wtime);
	printf("最后挂载时间:%d秒\n", superblock->s_mtime);
	printf("每个inode节点大小:%d\n", superblock->s_inode_size);
	return;
}

void read_file()
{
	struct ext3_dir_entry* dir = NULL;
	// 查找ext3fs目录
	dir = find_dirs(rootinode, 2, "ext3fs");
	if(dir == NULL)
	{
		printf("没有找到ext3fs目录\n");
		return;
	}
	// 显示ext3fs目录的目录项信息
	dump_one_dir(dir);
	// 查找ext3fs目录下的ext3.txt文件
	dir = find_dirs(get_inode(dir->inode), 1, "ext3.txt");
	if(dir == NULL)
	{
		printf("没有找到ext3.txt\n");
		return;
	}
	// 显示ext3.txt文件的目录项信息
	dump_one_dir(dir);
	// 显示ext3.txt文件的内容信息
	dump_inode_data(get_inode(dir->inode));
	return;
}

int main()
{
	if(init_in_hdfile() < 0)
	{
		printf("初始化失败\n");
		return -1;
	}
	dump_super_block(super);
	dump_all_group();
	dump_inode(rootinode);
	dump_dirs(rootinode);
	read_file();
	close(hdfilefd);
	return 0;
}