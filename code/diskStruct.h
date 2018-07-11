#ifndef __DISKSTRUCT_H__
#define __DISKSTRUCT_H__

#include "dataType.h"

#define IS_TRUE 1
#define IS_FALSE 0

// 硬盘设置
#define DISK "/home/doubibobo/ext2/ext2"
// 文件系统的状态
#define EXT2_VALID_FS 0X0001      // 文件系统没有出错
#define EXT2_ERROR_FS 0x0002      // 内核检测到错误
// inode节点指向设置，默认为9个直接地址，1个间接地址
// 间接规定一个文件大小的最大值为9k + 256k
#define EXT2_N_BLOCKS 10 

// 定义文件类型
#define ORDINARY_FILE 1
#define DIR_FILE = 2
#define BLOCK_FILE = 4
#define CHAR_FILE = 8
#define SOCKET_FILE = 16
#define SYMBOL_FILE = 32
#define FIFO_FILE = 64

// 定义访问许可
#define IRUSR 256   // 文件主可读
#define IWUSR 128    // 文件主可写  
#define IXUSR 64     // 文件主可执行 
#define IRGRP 32     // 同组用户可读 
#define IWGRP 16     // 同组用户可写
#define IXGRP 8      // 同组用户可执行
#define IROTH 4      // 其它用户可读
#define IWOTH 2      // 其它用户可写
#define IXOTH 1      // 其它用户可执行

// 定义引导块的长度
#define HOME_LENGTH 1024
// 定义超级块的长度
#define SUPER_BLOCK_LENGTH 1024
// 定义组描述的长度
#define GROUP_DESC_BLOCK_LENGTH 1024
// 定义块位图大小
#define BIT_MAP_SIZE 1024
// 定义索引位图大小
#define INDEX_MAP_SIZE 1024

// 定义外存inode节点的长度
#define OUT_INODE_LENGTH 128
// 定义目录项dentry的长度
#define DIR_DENTRY_LENGTH 256
// 定义每一块的长度
#define EVERY_BLOCK 1024
// 文件系统的总块数
#define BLOCKS_NUMBERS 8193

// 文件系统的第一个数据块位置，从0开始编号使然
#define FIRST_DATA_BLOCK 132

// 定义文件系统的总组数目
#define GROUP_NUMBER 8
// 定义每一组含有的块数
#define BOLOCKS_OF_EVERY_GROUP 1024

// 定义组描述表项大小
#define GROUP_DESC_SIZE 32

// 引导块数据结构
// 超级块大小为1K
struct ext2_super_block
{	
    __u32	s_inodes_count;		    /* 文件系统中索引节点总数 */
    __u32	s_blocks_count;			/* 文件系统中总块数 */
    __u32	s_r_blocks_count;	  	/* 为超级用户保留的块数 */
    __u32	s_free_blocks_count;	/* 文件系统中空闲块总数 */
    __u32	s_free_inodes_count;	/* 文件系统中空闲索引节点总数*/
    __u32	s_first_data_block;		/* 文件系统中第一个数据块 */
    __u32	s_log_block_size;	 	/* 用于计算逻辑块大小 */
    __s32	s_log_frag_size;	 	/* 用于计算片大小 */

    __u32	s_blocks_per_group;	    /* 每组中块数 */
    __u32	s_frags_per_group;  	/* 每组中片数 */
    __u32	s_inodes_per_group;	    /* 每组中索引节点数 */
    __u32	s_mtime;   			    /* 最后一次安装操作的时间 */
    __u32	s_wtime;  		        /* 最后一次对该超级块进行写操作的时间 */
    __u16	s_mnt_count;	        /* 安装计数 */
    __s16	s_max_mnt_count;	    /* 最大可安装计数 */
    __u16	s_magic;                /* 用于确定文件系统版本的标志 */
    __u16	s_state;                /* 文件系统的状态*/
    __u16	s_errors;               /* 当检测到有错误时如何处理 */
    __u16	s_minor_rev_level; 	    /* 次版本号 */
    __u32	s_lastcheck;	        /* 最后一次检测文件系统状态的时间 */
    __u32	s_checkinterval;        /* 两次对文件系统状态进行检测的间隔时间*/
    __u32	s_rev_level;	        /* 版本号 */
    __u16	s_def_resuid;	        /* 保留块的默认用户标识号 */
    __u16	s_def_resgid;           /* 保留块的默认用户组标识号*/	
    __u32   s_first_ino;            /* 第一个非保留的索引节点 */
    __u16   s_inode_size;           /* 索引节点的大小 */
    __u16   s_block_group_nr;       /* 该超级块的块组号 */
    __u32   s_feature_compat;       /* 兼容特点的位图*/
    __u32   s_feature_incompat;     /* 非兼容特点的位图 */
    __u32   s_feature_ro_compat;    /* 只读兼容特点的位图*/
    __u8    s_uuid[16];             /* 128位的文件系统标识号*/
    __u32   s_algorithm_usage_bitmap;  /* 用于压缩*/
    __u8    s_prealloc_blocks;      /* 预分配的块数*/
    __u8    s_prealloc_dir_blocks;  /* 给目录预分配的块数 */
    __u16   s_padding1;             /* 填充 */
    __u32   s_reserved[205];        /* 用null填充块的末尾 */
    char   s_volume_name[16];       /* 卷名 */
    char   s_last_mounted[64];      /* 最后一个安装点的路径名 */
};

// 组块描述符表项，总计占有32个字节大小
struct ext2_group_desc
{
	__u32 bg_block_bitmap;	 	/* 组中块位图所在的块号 */
	__u32 bg_inode_bitmap;  	/* 组中索引节点位图所在块的块号 */
	__u32 bg_inode_table;  		/* 组中索引节点表的首块号 */
	__u16 bg_free_blocks_count; /* 组中空闲块数 */
	__u16 bg_free_inodes_count; /* 组中空闲索引节点数 */
	__u16 bg_used_dirs_count;	/* 组中分配给目录的节点数 */
	__u16 bg_pad;  				/* 填充，对齐到字*/
	__u32 bg_reserved[3];   	/* 用null填充12个字节*/
};

// 组块描述符表，总计占有1024个字节大小
struct ext2_group_desc_table
{
    struct ext2_group_desc every[GROUP_NUMBER];
    __u8 padding[GROUP_DESC_BLOCK_LENGTH - GROUP_NUMBER*GROUP_DESC_SIZE];
};

// 块位图初始化结构体
struct ext2_bit_map
{
    char first[16];
    char second;
    char third[111];
};

// 外存索引节点，128个字节
struct ext2_out_inode {	
    // TODO __u16 i_mode; 			        /* 文件类型和访问权限 */
    __u8 i_type;                    /* 文件类型*/
    __u8 i_mode;                    /* 文件访问权限*/
	__u16 i_uid;				    /* 文件拥有者标识号*/
	__u32 i_size; 				    /* 以字节计的文件大小 */
	__u32 i_atime; 			        /* 文件的最后一次访问时间 */
	__u32 i_ctime; 			        /* 该节点最后被修改时间 */
	__u32 i_mtime; 			        /* 文件内容的最后修改时间 */
	__u32 i_dtime; 			        /* 文件删除时间 */
	__u16 i_gid;				    /* 文件的用户组标志符 */
	__u16 i_links_count; 		    /* 文件的硬链接计数 */
	__u32 i_blocks; 			    /* 文件所占块数（每块以512字节计）*/
    __u32 i_flags;				    /* 打开文件的方式 */
    union
    {  					            /* 特定操作系统的信息*/
        __u32 i_block[EXT2_N_BLOCKS];   /* 指向数据块的指针数组 */
        __u32 i_version;	  	        /* 文件的版本号（用于 NFS） */
        __u32 i_file_acl; 		        /* 文件访问控制表（已不再使用） */
        __u32 i_dir_acl;	 	        /* 目录访问控制表（已不再使用）*/
        __u8 l_i_frag;	 		        /* 每块中的片数 */
        __u32 i_faddr;  	            /* 片的地址 */
    };
    char padding[52];                   // 数据填充，为了对齐
};

// 目录项结构，256个字节
struct ext2_dir_entry_2 {
    __u32 inode;    // 文件入口的inode号，0表示该项未使用
    __u16 rec_len;  // 目录项长度
    __u8 name_len;  // 文件名包含的字符数
    __u8 file_type; // 文件类型
    char name[128]; // 文件名
    char padding[120];  // 字节对齐
};   



#endif