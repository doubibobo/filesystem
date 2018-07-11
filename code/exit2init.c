#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include "diskStruct.h"

// 读写磁盘文件指针
FILE *disk;

// 超级块定义
struct ext2_super_block super_block;

// 组描述符定义
struct ext2_group_desc group_init;

// 描述符表项
struct ext2_group_desc_table group_desc_table;

// 块位图初始化定义
struct ext2_bit_map bit_map;

// 引导块初始化
BOOL home_block_init()
{
    if ((disk = fopen(DISK, "r+")) == NULL)
    {
        printf("引导块初始化失败，请重新启动后重试\n");
        fclose(disk);
        return IS_FALSE;
    } else 
    {
        char home_buffer[1024] = {'0'};
        printf("磁盘引导区块大小为：%ld\n", sizeof(home_buffer));
        fclose(disk);
        return IS_TRUE;
    }
}

// 快组0的超级块初始化
BOOL group_super_block_init()
{
    super_block.s_inodes_count = 8192;          // 索引节点总数
    super_block.s_blocks_count = 8192;          // 文件系统总块数
    super_block.s_r_blocks_count = 64;          // 为超级块保留的块数    
    super_block.s_free_blocks_count = 8168;     // TODO 空闲物理块数
    super_block.s_free_inodes_count = 8168;     // TODO 空闲索引节点总数
    super_block.s_log_block_size = 0;           // 每一块的大小，此处为1024k
    super_block.s_first_data_block = FIRST_DATA_BLOCK;         // 文件系统中的第一个数据块, inode节点需要128个磁盘块，故开始的数据块为1+1+1+128 = 131块
    super_block.s_log_frag_size = 1;            // 片大小，此处不使用
    super_block.s_blocks_per_group = 1024;      // 每一个组中为1024个块，总共8个组
    super_block.s_frags_per_group = 0;          // 片数目，这里省略
    super_block.s_inodes_per_group = 0;         // 每个组中的索引节点数目为1024，同物理块保持一致
    super_block.s_mtime = 1;                    // TODO 赋值为当前时间
    super_block.s_wtime = 1;                    // TODO 最后一次安装操作的时间，也是当前时间
    super_block.s_mnt_count = 1;                // 安装计数
    super_block.s_max_mnt_count = 1;            // 最大可安装计数
    super_block.s_magic = 1;                    // 文件系统版本
    super_block.s_state = EXT2_VALID_FS;        // 文件系统的状态，初始化为正常
    super_block.s_errors = 1;                   // TODO 检测到错误时候的出错处理
    super_block.s_minor_rev_level = 1;          // 次版本号
    super_block.s_lastcheck = 1;                // TODO 最后一次检查的时间，设置为当前时间
    super_block.s_checkinterval = 1000;         // TODO 两次对文件系统进行状态检查的间隔时间
    super_block.s_rev_level = 1;                // TODO 版本号
    super_block.s_def_resuid = 1;               // TODO 保留快的默认用户标识号
    super_block.s_def_resgid = 1;               // TODO 保留快的默认用户组标识号
    super_block.s_first_ino = 1;                // TODO 第一个非保留的索引节点
    super_block.s_inode_size = 1;               // TODO 索引节点的大小
    super_block.s_block_group_nr = 0;           // 该超级块的块组号
    super_block.s_feature_compat = 0;           // TODO 兼容特点的位图
    super_block.s_feature_incompat = 0;         // TODO 非兼容特点的位图
    super_block.s_uuid[0] = 1;                  // TODO 128位的文件系统标识号
    strcpy(super_block.s_volume_name, "C:");    // 卷名
    strcpy(super_block.s_last_mounted, DISK);   // 最后一个安装点的路径名
    super_block.s_algorithm_usage_bitmap = 1;   // TODO 用于压缩
    super_block.s_prealloc_blocks = 1;          // 预分配的块数
    super_block.s_prealloc_dir_blocks = 1;      // 给目录预分配的块数
    super_block.s_padding1 = 0;                 // 填充
    super_block.s_reserved[0] = 1;              // 全部用0填充完一块

    // 将该结构体写入到文件中去
    if ((disk = fopen(DISK, "r+")) == NULL) 
    {
        // 如果以读写方式打开文件失败
        printf("组块超级块初始化失败！\n");
        return IS_FALSE;
    } else 
    {
        // 先找到超级块的位置，从文件的开始处偏移引导块大小的位置
        fseek(disk, HOME_LENGTH, SEEK_SET);
        if (fwrite(&super_block, sizeof(struct ext2_super_block), 1, disk))
        {
            printf("组块0超级块初始化成功！\n");
            fclose(disk);
            return IS_TRUE;
        } else
        {
            printf("组块0超级块初始化失败！\n");
            fclose(disk);
            return IS_FALSE;
        }
    }
}

// 对其他快组进行初始化，实质是复制组号0的超级块
BOOL other_group_super_block_init()
{
    if ((disk = fopen(DISK, "r+")) == NULL)
    {
        printf("其它组超级块初始化失败！\n");
        return IS_FALSE;
    } else 
    {
        int i;
        for (i = 1; i < GROUP_NUMBER; i++)
        {
            fseek(disk, HOME_LENGTH + BOLOCKS_OF_EVERY_GROUP*EVERY_BLOCK*i, SEEK_SET);
            if (fwrite(&super_block, sizeof(struct ext2_super_block), 1, disk))
            {
                printf("组块%d初始化成功!\n", i);
            } else 
            {
                printf("组块%d初始化失败!\n", i);
            }
        }
        fclose(disk);
        return IS_TRUE;
    }
}



// 对块组0的组块描述表初始化
BOOL group_desc_init()
{
    int i;
    for (i = 0; i < GROUP_NUMBER; i++)
    {
        group_desc_table.every[i].bg_block_bitmap = 3;             // 块位图所在块号
        group_desc_table.every[i].bg_inode_bitmap = 4;             // 索引节点位图所在块号
        group_desc_table.every[i].bg_inode_table = 5;              // 组中索引节点表的首块号
        group_desc_table.every[i].bg_free_blocks_count = 892;      // TODO 组中空闲块数
        group_desc_table.every[i].bg_free_inodes_count = 1024;     // TODO 组中空闲节点数目
        group_desc_table.every[i].bg_used_dirs_count = 64;         // TODO 组中分配给目录的节点数目
        group_desc_table.every[i].bg_pad = 0;                      // 填充，对齐到字
    }

    // 将结构体写入到文件当中去
    if ((disk = fopen(DISK, "r+")) == NULL)
    {
        printf("组块描述符初始化失败！\n");
        return IS_FALSE;
    } else 
    {
        // 先找到组块的位置，从文件的开始处偏移引导块大小的位置
        fseek(disk, HOME_LENGTH + SUPER_BLOCK_LENGTH, SEEK_SET);
        if (fwrite(&group_desc_table, sizeof(struct ext2_group_desc_table), 1, disk))
        {
            printf("组块0组块描述符初始化成功！\n");
            fclose(disk);
            return IS_TRUE;
        } else
        {
            printf("组块0组块描述符初始化失败！\n");
            fclose(disk);
            return IS_FALSE;
        }
    }
}

// 对其他块组的初始化操作
BOOL other_gropu_desc_init()
{
    // 将结构体写入到文件当中去
    if ((disk = fopen(DISK, "r+")) == NULL)
    {
        printf("组块0的组块描述符初始化失败！\n");
        return IS_FALSE;
    } else 
    {
        // 先找到组块的位置，从文件的开始处偏移引导块大小的位置
        int i;
        for (i = 1; i < GROUP_NUMBER; i++)
        {
            fseek(disk, HOME_LENGTH + SUPER_BLOCK_LENGTH + BOLOCKS_OF_EVERY_GROUP*EVERY_BLOCK*i, SEEK_SET);
            if (fwrite(&group_desc_table, sizeof(struct ext2_group_desc_table), 1, disk))
            {
                printf("组块%d组块描述符初始化成功！\n", i);
            } else
            {
                printf("组块%d组块描述符初始化失败！\n", i);
            }
        }
        fclose(disk);
        return IS_TRUE;
    }
}

// 块位图初始化
BOOL block_bitmap_init()
{
    // 总共8个组块，每一个组块用到128B，8个组块合计为128*8 = 1024B
    // 在每一个128B中，第一个B表示最前面的8块，最前面8块使用情况是
    // 1111
    // 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111  11111111 11111111 11111111 11111111
    // 128位1
    if ((disk = fopen(DISK, "r+")) == NULL)
    {
        printf("块位图（物理位图）初始化失败！\n");
        return IS_FALSE;
    } else 
    {
        int i, j;
        char pad1 = 255, pad2 = 240;
        for (i = 0; i < 16; i++)
        {
            bit_map.first[i] = pad1;
        }
        bit_map.second = pad2;
        for (i = 0; i < 111; i++)
        {
            bit_map.third[i] = 0;
        }
        for (i = 0; i < GROUP_NUMBER; i++)
        {
            fseek(disk, HOME_LENGTH + BOLOCKS_OF_EVERY_GROUP*EVERY_BLOCK*i + SUPER_BLOCK_LENGTH + GROUP_DESC_BLOCK_LENGTH, SEEK_SET);
            fwrite(&bit_map, sizeof(struct ext2_bit_map), GROUP_NUMBER, disk);
        }
        printf("块位图初始化成功!\n");
        return IS_TRUE;        
    }
}

// 索引位图初始化
BOOL index_bitmap_init()
{
    if ((disk = fopen(DISK, "r+")) == NULL)
    {
        printf("索引位图（逻辑位图）初始化失败！\n");
        return IS_FALSE;
    } else 
    {
        char index_bit[1024];
        int i;
        for (i = 0; i < 1024; i++)
        {
            index_bit[i] = 0;
        }
        for (i = 0; i < GROUP_NUMBER; i++)
        {
            fseek(disk, HOME_LENGTH + BOLOCKS_OF_EVERY_GROUP*EVERY_BLOCK*i + SUPER_BLOCK_LENGTH + GROUP_DESC_BLOCK_LENGTH + BIT_MAP_SIZE, SEEK_SET);
            fwrite(&index_bit, sizeof(index_bit), 1, disk);
        }
        printf("索引位图（逻辑位图）初始化成功！\n");
        return IS_TRUE;
    }
}

// 外存inode节点初始化
void out_inode_table_init()
{

}

// 文件系统初始化函数
void ext2_init() 
{
    // 引导块初始化
    if (home_block_init()) 
    {
        printf("引导块初始化成功！\n");
    }
    // 超级块初始化，初始化组块0
    if (group_super_block_init())
    {
        printf("组块0超级块初始化成功！\n");
    }
    // 其他块需要进行复制，作为其副本
    if (other_group_super_block_init())
    {
        printf("其它组块初始化成功！\n");
    }
    // 组块描述表初始化
    if (group_desc_init()) 
    {
        printf("组块0组描述符初始化成功！\n");
    }
    // 其他块组的组块描述符需要进行复制，作为其副本
    if (other_gropu_desc_init())
    {
        printf("其它组块描述符初始化成功！\n");
    }
    // 块位图初始化，占1k大小
    if (block_bitmap_init())
    {
        printf("块位图初始化成功！\n");
    }
    // 索引位图初始化，占1k大小
    if (index_bitmap_init())
    {
        printf("索引位图初始化成功！\n");
    }
    // 索引表初始化
    out_inode_table_init();
}

// 硬盘检查,以只读方式打开
BOOL disk_check()
{
   FILE *fp;
   if ((fp = fopen(DISK, "r")) == NULL)
   {
       printf("磁盘不存在，请检查磁盘后进行操作\n");
       fclose(fp);
       return IS_FALSE;
   } else 
   {
       printf("磁盘检查正常，现在开始加载文件系统\n");
       fclose(fp);
       return IS_TRUE;
   }
}

int main()
{
    struct ext2_super_block example;
    struct ext2_group_desc group_desc_example;
    struct ext2_out_inode out_inode;
    struct ext2_dir_entry_2 dentry;
    struct ext2_bit_map bit_map;
    struct ext2_group_desc_table group_desc_table;
    if (disk_check())
    {
        ext2_init();
    }
    printf("super_block:%ldB\n", sizeof(example));
    printf("group_desc_block:%ldB\n", sizeof(group_desc_example));
    printf("%ld\n", sizeof(example) + sizeof(group_desc_example));
    printf("%ld\n", sizeof(out_inode));
    printf("%ld\n", sizeof(dentry));
    printf("%ld\n", sizeof(bit_map));
    printf("%ld\n", sizeof(group_desc_table));
    return 0;
}