#include "memory.h"

FILE* out_disk;

// 定义内存超级块
struct ext2_in_super_block super_block_in;

// 定义内存inode节点链表
struct ext2_in_inode in_inode_array[NR_OPEN];

// 定义系统打开文件表
struct file system_file[NR_OPEN];

// 定义用户打开文件表
struct files_struct user_file[NR_OPEN][NR_OPEN];

// 当前的进程数目，默认为0
__u16 task_number = 0;

// 当前内存中含有的inode数目，默认为0
__u16 current_inode_number = 0;

// 当前打开文件的数目
__u16 current_open_file = 0;

// 定义当前的inode编号
__u16 current_inode;

// 定义当前的工作目录
char* current_dir;

// 将磁盘超级块中的内容复制到内存超级块中
BOOL super_block_out_to_in()
{
    if ((out_disk = fopen(DISK, "r+")) != NULL)
    {
        struct ext2_super_block origin;
        fseek(out_disk, HOME_LENGTH, SEEK_SET);
        // 将磁盘超级块读取到内存超级块中
        if (fread(&origin, SUPER_BLOCK_LENGTH, 1, out_disk))
        {
            // 将origin中的内容复制到内存超级块中
            super_block_in.count = 1;                       // 定义访问计数
            super_block_in.flag = NOT_MODIFIED;             // 使用状态
            super_block_in.super_dev = 1;                   // 所在设备的设备号，默认为1号设备
            super_block_in.origin = origin;
            // 内存超级块加载成功！
            printf("\n\n");
            printf("%d, %d, %s\n", super_block_in.origin.s_free_blocks_count, super_block_in.origin.s_free_inodes_count, super_block_in.origin.s_last_mounted);
            printf("%ld\n", sizeof(super_block_in));
            fclose(out_disk);
            return IS_TRUE;
        }
        fclose(out_disk);
    }
    return IS_FALSE;
}

// 将修改后的超级块回写到磁盘
BOOL super_block_in_to_out()
{
    if ((out_disk = fopen(DISK, "r+")) != NULL)
    {
        struct ext2_super_block origin;
        fseek(out_disk, HOME_LENGTH, SEEK_SET);
        if (fwrite(&origin, SUPER_BLOCK_LENGTH, 1, out_disk))
        {
            printf("内存超级块回写成功！\n");
            fclose(out_disk);
            return IS_TRUE;
        }
        fclose(out_disk);
    }
    return IS_FALSE;
}

// 将已经使用的内存inode节点表项从磁盘中复制到内存，并且形成一个链表，从0开始，group是组号
BOOL used_inode_out_to_in(__u16 number, __u8 group)
{
    if (current_inode_number >= NR_OPEN)
    {
        printf("超出用户最大打开的文件数目！\n");
        return IS_FALSE;
    }
    if ((out_disk = fopen(DISK, "r+")) != NULL)
    {
        // 首先找到磁盘空间中编号为number的内存节点的位置
        __u32 begin_first = FIRST_INODE_BLOCK*EVERY_BLOCK  + EVERY_GROUP_INODE_BLOCK*EVERY_BLOCK*group + OUT_INODE_LENGTH*number;
        struct ext2_out_inode origin;
        fseek(out_disk, begin_first, SEEK_SET);
        if (fread(&origin, OUT_INODE_LENGTH, 1, out_disk))
        {
            printf("文件加载成功！\n");
            printf("%d\n", origin.i_block[0]);
            // 将origin填到内存inode中去
            in_inode_array[number].origin = origin;
            in_inode_array[number].i_count++;
            in_inode_array[number].i_dev = super_block_in.super_dev;
            in_inode_array[number].i_flag = NOT_MODIFIED;
            current_inode_number++;
            fclose(out_disk);
            return IS_TRUE;
        }
        fclose(out_disk);
    }
    return IS_FALSE;
}

// 更改修改后的inode节点，并将其写回磁盘
BOOL used_inode_in_to_out(__u16 number, __u8 group)
{
    // 首先看其引用数是否为0， 当其为0的时候才可以回写
    if (in_inode_array[number].i_count != 0)
    {
        printf("该节点还在使用中，不能进行回写操作！\n");
        return IS_FALSE;
    }
    if ((out_disk = fopen(DISK, "r+")) != NULL)
    {
        // 首先找到磁盘空间中编号为number的内存节点的位置
        __u32 begin_first = FIRST_INODE_BLOCK*EVERY_BLOCK  + EVERY_GROUP_INODE_BLOCK*EVERY_BLOCK*group + OUT_INODE_LENGTH*number;
        struct ext2_out_inode origin = in_inode_array[number].origin;
        fseek(out_disk, begin_first, SEEK_SET);
        if (fwrite(&origin, OUT_INODE_LENGTH, 1, out_disk))
        {

            printf("inode回写成功！\n");
            printf("%d\n", origin.i_block[0]);
            // 将origin填到内存inode中去，将内存inode节点置为无效
            in_inode_array[number].i_count--;
            current_inode_number--;
            fclose(out_disk);
            return IS_TRUE;
        }
        fclose(out_disk);
        printf("inode节点回写失败！\n");
        return IS_FALSE;
    }
    printf("磁盘操作错误，inode节点回写失败！\n");
    return IS_FALSE;
}

// 填充系统打开文件表，number为inode节点的编号，从0开始
BOOL add_system_file(__u16 number, __u8 mode)
{   
    if (number >= NR_OPEN)
    {
        printf("文件系统内部错误！\n");
        return IS_FALSE;
    }
    system_file[number].f_mode = mode;
    system_file[number].pos = SEEK_SET;        // 定义文件的读写位置为文件的开始
    system_file[number].f_count = 1;
    system_file[number].f_inode = &in_inode_array[number];
    current_open_file++;
    return IS_TRUE;
}

// 删除用户打开文件表
BOOL delete_system_file(__u16 number)
{
    if (number >= NR_OPEN)
    {
        printf("文件系统内部错误！\n");
        return IS_FALSE;
    } 
    if (system_file[number].f_count > 1)
    {
        printf("其他进程正在占用该文件，不能进行删除系统打开文件表项操作");
        return IS_FALSE;
    }
    current_open_file--;
    used_inode_in_to_out(number, 0);
    system_file[number].f_inode = NULL;
}

// 添加用户打开文件表表项
BOOL add_files_struct(__u8 the_task, __u8 the_inode)
{
    // 定义
    user_file[the_task][the_inode].fd[0] = NULL;
    return IS_TRUE;
}

// 删除用户打开文件表
BOOL delete_files_struct()
{
    return IS_TRUE;
}

// 将文件系统加载到内存
BOOL ext2_system_out_to_in()
{
    return IS_TRUE;
}

// 获取当前工作路径
char* get_pwd()
{
    return current_dir;
}