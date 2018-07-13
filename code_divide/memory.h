#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>

#include "memoryStruct.h"

// 将磁盘超级块中的内容复制到内存超级块中
BOOL super_block_out_to_in();

// 将修改后的超级块回写到磁盘
BOOL super_block_in_to_out();

// 将已经使用的内存inode节点表项从磁盘中复制到内存，并且形成一个链表
BOOL used_inode_out_to_in(__u16 number, __u8 group);

// 更改修改后的inode节点，并将其写回磁盘
BOOL used_inode_in_to_out(__u16 number, __u8 group);

// 填充系统打开文件表
BOOL add_system_file(__u16 number, __u8 mode);

// 删除用户打开文件表
BOOL delete_system_file(__u16 number);

// 填充用户打开文件表
BOOL add_files_struct(__u8 the_task, __u8 the_inode);

// 删除用户打开文件表
BOOL delete_files_struct();

// 将文件系统加载到内存
BOOL ext2_system_out_to_in();

// 获取当前工作路径
char* get_pwd();

#endif