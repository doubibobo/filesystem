#ifndef __EXIT2INIT_H
#define __EXIT2INIT_H

#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>

#include "dataType.h"
#include "diskStruct.h"

// 引导块初始化
BOOL home_block_init();

// 快组0的超级块初始化
BOOL group_super_block_init();

// 对其他快组进行初始化，实质是复制组号0的超级块
BOOL other_group_super_block_init();

// 对块组0的组块描述表初始化
BOOL group_desc_init();
// 对其他块组的初始化操作
BOOL other_gropu_desc_init();

// 块位图初始化
BOOL block_bitmap_init();

// 索引位图初始化
BOOL index_bitmap_init();

// 采用整块的分配方式，获取第一个空闲数据块的编号，注意空闲数据块从0开始编号，返回空闲块的块号
__u16 get_one_free_block_bitmap();

// 设置块位图当中的某一位，默认为0号组，默认偏移量为0，即从开始处进行写
BOOL set_one_bit_of_block_bitmap(int offset, int value, int number);

// 获取第一个空闲inode
__u16 get_one_free_index_bitmap();

// 设置索引位图中的某一位
BOOL set_one_bit_of_index_bitmap(int offset, int value, int number);

// 外存inode节点初始化，创建根目录以及root目录
BOOL out_inode_table_init();

// 文件系统初始化函数
void ext2_init();

// 硬盘检查,以只读方式打开
BOOL disk_check();

#endif