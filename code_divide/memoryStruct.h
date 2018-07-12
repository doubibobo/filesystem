#ifndef __MEMORYSTRUCT_H__
#define __MEMORYSTRUCT_H__

#include "dataType.h"
#include "diskStruct.h"

// 定义用户最多打开的文件数目
#define NR_OPEN 256

// 定义文件打开后的处理方式
#define O_RDONLY    1   // 只读
#define O_WRONLY    2   // 只写
#define O_RDWR      3   // 读写
#define O_APPEND    4   // 附加写
#define O_CRETAE    5   // 如果不存在则进行创建，配合mode参数
#define O_EXCL      6   // 用于原子操作，配合O_CREATE如果不存在则创建
#define O_TRUNC     7   // 如果只读只写，则长度截为0
#define O_NOCTTY    8   // 不作为控制终端
#define O_NONBLOCK  9   // 非阻塞
#define O_SYNC      10  // 等待磁盘IO完成

// 定义文件创建或打开时指定的文件属性
#define FMODE_READ  1   // 读
#define FMODE_WRITE 2   // 写

// 定义对设备读写操作
#define SEL_IN   1  // 从设备读取
#define SEL_OUT  2  // 向设备写入

// 定义内存超级块
struct ext2_in_super_block
{
    // 新添加的信息
    __u8 super_dev;         // 所在设备设备号
    __u8 count;             // 访问计数
    __u8 flag;              // 使用状态

    // 原始信息
    struct ext2_super_block origin;
};

// 定义内存inode节点
struct ext2_in_inode 
{
    // 新添加的信息
    __u8 i_dev;             // 设备号
    __u8 i_count;           // 节点引用计数
    __u8 i_flag;            // 设置inode节点状态
    struct ext2_in_inode* i_forw;   // inode前向指针
    struct ext2_in_inode* i_back;   // inode后向指针

    // 原始信息     
    struct ext2_out_inode origin;     
};

// 定义系统打开文件表项
struct file 
{
    __u8 f_mode;                        // 文件的打开模式
    __u64 pos;                          // 文件的当前读写位置
    __u16 f_flags;                      // 文件操作标志
    __u16 f_count;                      // 共享该结构体的进程计数值
    __u64 f_reada;  
    __u64 f_ramax;
    __u64 f_raend;
    __u64 f_ralen;
    __u64 f_rawin;
    __u64 f_owner;                      // SIGIO用PID
	__u64 f_version;                    // 文件版本

    struct file* f_next;                // 此项的后继指针
    struct file* f_pre;                 // 此项的前驱指针
    struct inode* f_inode;              // 指向文件对应的inode

	struct file_operations * f_op;      // 指向文件操作结构体的指针
	void *private_data;                 // 指向与文件管理模块有关的私有数据的指针
};

// 定义用户打开文件表
struct files_struct
{
    __u32 count;                        // 共享该结构的计数值
    __u8 close_on_exec;                 // 执行关闭标志(close-on-exec)，其值为0（缺省）表示当前进程执行一目标文件时不关闭这个打开文件，值为1时表示当前进程执行目标文件的时候关闭这个文件
    __u8 open_fds;                      
    struct file* fd[NR_OPEN];           
    // 0定向为标准输入  键盘输入——可以重定向为任一文件
    // 1定向为标准输出  屏幕输出——可以重定向为任一文件
    // 2出错输出    指定为屏幕输出，不可以重定向      
};

// 定义超级块的操作函数
struct super_operations {
	void (*read_inode) (struct ext2_in_inode *);
	int (*notify_change) (struct ext2_in_inode *, struct iattr *);
	void (*write_inode) (struct ext2_in_inode *);
	void (*put_inode) (struct ext2_in_inode *);
	void (*put_super) (struct ext2_in_super_block *);
	void (*write_super) (struct ext2_in_super_block *);
	void (*statfs) (struct ext2_in_super_block *, struct statfs *, int);
	int (*remount_fs) (struct ext2_in_super_block *, int *, char *);
};

// 定义内存inode操作函数
struct inode_operations {
	struct file_operations * default_file_ops;
	int (*create) (struct ext2_in_inode *,const char *,int,int,struct ext2_in_inode **);
	int (*lookup) (struct ext2_in_inode *,const char *,int,struct ext2_in_inode **);
	int (*link) (struct ext2_in_inode *,struct ext2_in_inode *,const char *,int);
	int (*unlink) (struct ext2_in_inode *,const char *,int);
	int (*symlink) (struct ext2_in_inode *,const char *,int,const char *);
	int (*mkdir) (struct ext2_in_inode *,const char *,int,int);
	int (*rmdir) (struct ext2_in_inode *,const char *,int);
	int (*mknod) (struct ext2_in_inode *,const char *,int,int,int);
	int (*rename) (struct ext2_in_inode *,const char *,int,struct ext2_in_inode *,const char *,int, int);
	int (*readlink) (struct ext2_in_inode *,char *,int);
	int (*follow_link) (struct ext2_in_inode *,struct ext2_in_inode *,int,int,struct ext2_in_inode **);
	int (*readpage) (struct ext2_in_inode *, struct page *);
	int (*writepage) (struct ext2_in_inode *, struct page *);
	int (*bmap) (struct ext2_in_inode *,int);
	void (*truncate) (struct ext2_in_inode *);
	int (*permission) (struct ext2_in_inode *, int);
	int (*smap) (struct ext2_in_inode *,int);
};

// 定义文件系统的操作函数结构体
struct file_operations {
	int (*lseek) (struct ext2_in_inode *, struct file *, __u64, int);
	int (*read) (struct ext2_in_inode *, struct file *, char *, int);
	int (*write) (struct ext2_in_inode *, struct file *, const char *, int);
	int (*readdir) (struct ext2_in_inode *, struct file *, void *, int);
	int (*select) (struct ext2_in_inode *, struct file *, int, int);
	int (*ioctl) (struct ext2_in_inode *, struct file *, unsigned int, unsigned long);
	int (*mmap) (struct ext2_in_inode *, struct file *, struct vm_area_struct *);
	int (*open) (struct ext2_in_inode *, struct file *);
	void (*release) (struct ext2_in_inode *, struct file *);
	int (*fsync) (struct ext2_in_inode *, struct file *);
	int (*fasync) (struct ext2_in_inode *, struct file *, int);
	int (*check_media_change) (int dev);
	int (*revalidate) (int dev);
};

#endif