#include "exit2init.h"

// 读写磁盘文件指针
FILE *disk;

// 超级块定义
struct ext2_super_block super_block;

// 组描述符表项目定义
struct ext2_group_desc group_init;

// 描述符表
struct ext2_group_desc_table group_desc_table;

// 块位图初始化定义
struct ext2_bit_map bit_map;

// 根节点初始化定义
struct ext2_out_inode root_disk;

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
        // 设置0号inode节点不使用
        index_bit[0] = 0x80;
        for (i = 1; i < 1024; i++)
        {
            index_bit[i] = 0;
        }
        for (i = 0; i < GROUP_NUMBER; i++)
        {
            fseek(disk, HOME_LENGTH + BOLOCKS_OF_EVERY_GROUP*EVERY_BLOCK*i + SUPER_BLOCK_LENGTH + GROUP_DESC_BLOCK_LENGTH + BIT_MAP_SIZE, SEEK_SET);
            fwrite(&index_bit, sizeof(index_bit), 1, disk);
        }
        printf("索引位图（逻辑位图）初始化成功！\n");
        fclose(disk);
        return IS_TRUE;
    }
}

// 采用整块的分配方式，获取第一个空闲数据块的编号，注意空闲数据块从0开始编号，返回空闲块的块号
__u16 get_one_free_block_bitmap()
{
    // 首先获取块位图的开始
    __u32 first_begin = HOME_LENGTH + SUPER_BLOCK_LENGTH + GROUP_DESC_BLOCK_LENGTH;
    __u32 current = first_begin;
    if ((disk = fopen(DISK, "r+")) != NULL)
    {
        fseek(disk, current, SEEK_SET);
        for (; current < BIT_MAP_SIZE*8;)
        {
            __u8 current_char = fgetc(disk);
            if (current_char == 255)
            {
                current += 8;
            } else 
            {
                while (current_char & 0x80)
                {
                    current_char = current_char << 1;
                    ++current;
                }
                // 找到第一个空闲数据块
                printf("找到第一个空闲数据块，编号为%d\n", current - first_begin);
                return current - first_begin;
            }
        }
        printf("获取数据块失败，当前已经没有数据块可用！\n");
        return 0;
    }
    printf("获取数据块失败！\n");
    return 0;
}

// 设置块位图当中的某一位，默认为0号组，默认偏移量为0，即从开始处进行写
BOOL set_one_bit_of_block_bitmap(int offset, int value, int number)
{
    // 检查偏移量
    if (offset >= BIT_MAP_SIZE)
    {
        printf("位图越界！\n");
        return IS_FALSE;
    }
    // 首先获取块位图的开始
    __u32 first_begin = HOME_LENGTH + SUPER_BLOCK_LENGTH + GROUP_DESC_BLOCK_LENGTH + BLOCK_INDEX_BMP_SIZE*number;
    __u32 current = first_begin + offset / 8;
    // 打开文件以取出一位
    if ((disk = fopen(DISK, "r+")) != NULL) 
    {
        fseek(disk, current, SEEK_SET);             // 从初始位置开始偏移
        __u8 current_char = fgetc(disk);
        __u8 char_temp;
        if (offset % 8)
        {
            char_temp = current_char >> (8 - offset % 8 + 1);
            current_char = current_char << (offset % 8 - 1);
        }
        __u8 current_bit = current_char & 0x80;

        if (value && current_bit == 0) 
        {
            // 如果value的值为1且当前的位标志为0，则讲其进行置位操作
            current_char = current_char | 0x80;
        } else if (value == 0 || current_bit)
        {
            // 如果value的值为0且当前的位标志为1，则将其进行置位操作
            current_char = current_char & 0x7F;
        } else
        {
            // 上述两种情况都不满足，出现错误
            printf("块（物理块位图）操作失败！\n");
            fclose(disk);
            return IS_FALSE;
        }
        if (offset % 8)
        {
            current_char = current_char >> (offset % 8 - 1);
            char_temp = char_temp << (8 - offset % 8 + 1);
            current_char = current_char + char_temp;
        }
        // 向磁盘块写入一个字节的内容
        // 由于刚刚读出一个字符的时候，读写指针已经移动，所以要重新寻找相关地址
        fseek(disk, current, SEEK_SET);
        if (fputc(current_char, disk) != EOF)
        {
            printf("设置块位图成功！\n");
            // 修改超级块、组描述符中的空闲节点信息
            super_block.s_free_blocks_count--;
            group_desc_table.every[number].bg_free_blocks_count--;
            fclose(disk);
            return IS_TRUE;
        } else 
        {
            printf("设置块位图失败！\n");
            fclose(disk);
            return IS_FALSE;
        }
    }
    printf("块位图操作失败！\n");
    fclose(disk);
    return IS_FALSE;
}

// 获取第一个空闲inode
__u16 get_one_free_index_bitmap()
{
    // 首先获取索引位图的开始
    __u32 first_begin = HOME_LENGTH + SUPER_BLOCK_LENGTH + GROUP_DESC_BLOCK_LENGTH + BIT_MAP_SIZE;
    __u32 current = first_begin;
    if ((disk = fopen(DISK, "r+")) != NULL)
    {
        fseek(disk, current, SEEK_SET);
        for (; current < INDEX_MAP_SIZE*8; )
        {
            __u8 current_char = fgetc(disk);
            printf("current_char = %d\n", current_char);
            if (current_char == 255)
            {
                current += 8;
            } else 
            {
                while (current_char & 0x80)
                {
                    current_char = current_char << 1;
                    ++current;
                }
                // 找到第一个空闲数据块
                printf("找到第一个空闲inode，编号为%d\n", current - first_begin);
                return current - first_begin + 1;
            }
        }
        printf("获取索引节点失败，当前已经没有inode节点可用\n");
        return 0;
    }
    printf("获取索引节点失败！\n");
    return 0;
}

// 设置索引位图中的某一位
BOOL set_one_bit_of_index_bitmap(int offset, int value, int number)
{
    // 检查偏移量
    if (offset >= INDEX_MAP_SIZE)
    {
        printf("索引位图操作越界！\n");
        return IS_FALSE;
    }
    // 首先获取索引位图的开始
    __u32 first_begin = HOME_LENGTH + SUPER_BLOCK_LENGTH + GROUP_DESC_BLOCK_LENGTH + BIT_MAP_SIZE + BLOCK_INDEX_BMP_SIZE*number;
    __u32 current = first_begin + offset / 8;
    // 打开文件取出当中的某一位
    if ((disk = fopen(DISK, "r+")) != NULL)
    {
        fseek(disk, current, SEEK_SET);             // 从初始位置开始
        __u8 current_char = fgetc(disk);
        __u8 char_temp;
        if (offset % 8)
        {
            char_temp = current_char >> (8 - offset % 8 + 1);
            current_char = current_char << (offset % 8 - 1);
        }
        
        __u8 current_bit = current_char & 0x80;
        if (value && current_bit == 0)
        {
            current_char = current_char | 0x80;
        } else if (value == 0 || current_bit)
        {
            current_char = current_char & 0x7F;
        } else
        {
            printf("索引位图（索引位图）操作失败");
            fclose(disk);
            return IS_FALSE;
        }
        
        if (offset % 8)
        {
            current_char = current_char >> (offset % 8 - 1);
            char_temp = char_temp << (8 - offset % 8 + 1);
            current_char = current_char + char_temp;
        }

        // 向磁盘块写入一个字节的内容
        // 由于刚刚读出一个字符的时候，读写指针已经移动，所以要重新寻找相关地址
        fseek(disk, current, SEEK_SET);
        if (fputc(current_char, disk) != EOF)
        {
            printf("设置索引位图成功！\n");
            // 修改超级块、组描述符中的空闲节点信息
            super_block.s_free_inodes_count--;
            group_desc_table.every[number].bg_free_inodes_count--;
            fclose(disk);
            return IS_TRUE;
        } else 
        {
            printf("设置索引位图失败！\n");
            fclose(disk);
            return IS_FALSE;
        }
    }
}

// 外存inode节点初始化，创建根目录以及root目录
BOOL out_inode_table_init()
{
    // 定义根目录的权限
    __u16 permit = 0x1111;
    root_disk.i_type = DIR_FILE;
    root_disk.i_mode = permit;
    root_disk.i_uid = 0;                // 文件拥有者0
    
    // 获取当前时间戳
    __u32 current_time;
    time_t t = time(NULL);
    current_time = time(&t);
    root_disk.i_atime = current_time;
    root_disk.i_ctime = current_time;
    root_disk.i_mtime = current_time;
    root_disk.i_dtime = 0xFFFF;

    root_disk.i_gid = 0;                        // 文件用户组标识符
    root_disk.i_links_count = 1;                // 硬链接计数，初始值为0，创建root目录之后为1
    root_disk.i_blocks = 1;                     // 文件所占块数，初始化为1
    root_disk.i_flags = 3;                      // 设置文件打开方式为读写
    root_disk.i_block[0] = 0;                   // 设置文件系统的第一个数据块

    // 设置索引位图当中的某一位
    // 获取第一个inode节点，讲其编号为1，注意，inode节点编号为0的不用
    // 设置索引位图当中的某一位
    __u16 inode_number = get_one_free_index_bitmap();
    if (inode_number && set_one_bit_of_index_bitmap(inode_number, BLOCK_INDEX_IN_USE, 0))
    {
        struct ext2_dir_entry_2 root_disk_dentry_one_dot, root_disk_dentry_another_dot;
        root_disk_dentry_one_dot.inode = inode_number;
        root_disk_dentry_one_dot.rec_len = 256;             // 目录项所占空间大小为256个字节
        root_disk_dentry_one_dot.file_type = DIR_FILE;      // 定义文件类型为目录文件
        strcpy(root_disk_dentry_one_dot.name, ".");
        root_disk_dentry_one_dot.name_len = 1;

        root_disk_dentry_another_dot.inode = inode_number;
        root_disk_dentry_another_dot.rec_len = 256;             // 目录项所占空间大小为256个字节
        root_disk_dentry_another_dot.file_type = DIR_FILE;      // 定义文件类型为目录文件
        strcpy(root_disk_dentry_another_dot.name, "..");
        root_disk_dentry_another_dot.name_len = 2;

        // 创建root目录
        __u16 root_inode_number = get_one_free_index_bitmap();
        if (root_inode_number && set_one_bit_of_index_bitmap(root_inode_number, BLOCK_INDEX_IN_USE, 0))
        {
            struct ext2_out_inode root_dir;
            root_dir.i_type = 0x1111;
            root_dir.i_mode = permit;
            root_dir.i_uid = 1;

            __u32 root_dir_make_time;
            time_t temp = time(NULL);
            root_dir_make_time = time(&temp);
            root_dir.i_atime = root_dir_make_time;
            root_dir.i_ctime = root_dir_make_time;
            root_dir.i_mtime = root_dir_make_time;
            root_dir.i_dtime = 0xFFFF;

            root_dir.i_gid = 0;
            root_dir.i_links_count = 1;
            root_dir.i_blocks = 1;
            root_dir.i_flags = 3;
            root_dir.i_block[0] = 4;

            struct ext2_dir_entry_2 root_dir_dentry;
            root_dir_dentry.inode = root_inode_number;
            root_dir_dentry.rec_len = 256;             // 目录项所占空间大小为256个字节
            root_dir_dentry.file_type = DIR_FILE;      // 定义文件类型为目录文件
            strcpy(root_dir_dentry.name, "root");
            root_dir_dentry.name_len = 4;

            // 设置root的根目录
            struct ext2_dir_entry_2 root_dir_dentry_one_dot, root_dir_dentry_another_dot;
            root_dir_dentry_one_dot.inode = root_inode_number;
            root_dir_dentry_one_dot.rec_len = 256;             // 目录项所占空间大小为256个字节
            root_dir_dentry_one_dot.file_type = DIR_FILE;      // 定义文件类型为目录文件
            strcpy(root_dir_dentry_one_dot.name, ".");
            root_dir_dentry_one_dot.name_len = 1;

            root_dir_dentry_another_dot.inode = inode_number;
            root_dir_dentry_another_dot.rec_len = 256;             // 目录项所占空间大小为256个字节
            root_dir_dentry_another_dot.file_type = DIR_FILE;      // 定义文件类型为目录文件
            strcpy(root_dir_dentry_another_dot.name, "..");
            root_dir_dentry_another_dot.name_len = 2;

            if ((disk = fopen(DISK, "r+")) != NULL)
            {
                // 向磁盘写入inode节点
                fseek(disk, FIRST_INODE_BLOCK*EVERY_BLOCK, SEEK_SET);
                fwrite(&root_disk, OUT_INODE_LENGTH, 1, disk);
                fseek(disk, FIRST_INODE_BLOCK*EVERY_BLOCK + OUT_INODE_LENGTH, SEEK_SET);
                fwrite(&root_dir, OUT_INODE_LENGTH, 1, disk);
                // 向磁盘写入根目录的dentry目录
                fseek(disk, FIRST_DATA_BLOCK*EVERY_BLOCK, SEEK_SET);
                fwrite(&root_disk_dentry_one_dot, DIR_DENTRY_LENGTH, 1, disk);
                fseek(disk, FIRST_DATA_BLOCK*EVERY_BLOCK + DIR_DENTRY_LENGTH, SEEK_SET);
                fwrite(&root_disk_dentry_another_dot, DIR_DENTRY_LENGTH, 1, disk);
                fseek(disk, FIRST_DATA_BLOCK*EVERY_BLOCK + 2*DIR_DENTRY_LENGTH, SEEK_SET);
                fwrite(&root_dir_dentry, DIR_DENTRY_LENGTH, 1, disk);
                // 向磁盘写入root目录的dentry目录
                fseek(disk, (FIRST_DATA_BLOCK + 4)*EVERY_BLOCK, SEEK_SET);
                fwrite(&root_dir_dentry_one_dot, DIR_DENTRY_LENGTH, 1, disk);
                fseek(disk, (FIRST_DATA_BLOCK + 4)*EVERY_BLOCK + DIR_DENTRY_LENGTH, SEEK_SET);
                fwrite(&root_dir_dentry_another_dot, DIR_DENTRY_LENGTH, 1, disk);
                // 设置数据块被占用的标志
                printf("空闲块数目：%d\n", get_one_free_block_bitmap());
                set_one_bit_of_block_bitmap(FIRST_DATA_BLOCK + 1, BLOCK_INDEX_IN_USE, 0);
                printf("空闲块数目：%d\n", get_one_free_block_bitmap());
                set_one_bit_of_block_bitmap(FIRST_DATA_BLOCK + 2, BLOCK_INDEX_IN_USE, 0);
                printf("空闲块数目：%d\n", get_one_free_block_bitmap());
                printf("文件系统初始化完成，根目录以及root目录已经创建！\n");
                fclose(disk);
                return IS_TRUE;
            }
        }
    }    
    printf("硬盘操作失败！\n");
    return IS_FALSE;
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
    // 索引表初始化，创建/(根目录)和/root（root目录）
    if (out_inode_table_init())
    {
        printf("创建根目录以及root目录成功！\n");
    }
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