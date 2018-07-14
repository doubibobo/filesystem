#include "foperator.h"

__u16 count = 0;

FILE* current_disk;

// 创建文件
BOOL createFile(char* filename, __u8 type, char* current)
{   // 首先对当前目录进行处理，找到dentry目录所在的数据块
    // 向此dentry目录中添加一项
    // 根据当前文件路径，找到对应的inode节点
    __u16 current_inode = findInodeByDirFilename(current);
    if (current_inode == 0)
    {   // 没有找到对应的inode节点
        printf("创建文件失败，文件系统错误！\n");
        return IS_FALSE;
    } else 
    {
        // 找到了对应的inode节点，从文件中读取该节点的内容
        struct ext2_out_inode now_inode;
        if ((current_disk = fopen(DISK, "r+")) != NULL)
        {
            fseek(current_disk, HOME_LENGTH + FIRST_INODE_BLOCK*EVERY_BLOCK + OUT_INODE_LENGTH * (current_inode - 1), SEEK_SET);
            if (!fread(&now_inode, OUT_INODE_LENGTH, 1, current_disk))
            {
                printf("读文件失败！\n");
                return IS_FALSE;
            }
            // 找到第一个空闲inode节点
            fclose(current_disk);
            __u16 free_inode = get_one_free_index_bitmap();
            printf("free_inode = %d\n", free_inode);
            if (free_inode && set_one_bit_of_index_bitmap(free_inode, BLOCK_INDEX_IN_USE, 0))
            {   // 修改原来的inode
                // 设置inode节点
                // 定义根目录的权限
                struct ext2_out_inode free_out_inode;
                __u16 permit = 0x1111;
                free_out_inode.i_type = DIR_FILE;
                free_out_inode.i_mode = permit;
                free_out_inode.i_uid = 0;                // 文件拥有者0
                
                // 获取当前时间戳
                __u32 current_time;
                time_t t = time(NULL);
                current_time = time(&t);
                free_out_inode.i_atime = current_time;
                free_out_inode.i_ctime = current_time;
                free_out_inode.i_mtime = current_time;
                free_out_inode.i_dtime = 0xFFFF;
                free_out_inode.i_gid = 0;                        // 文件用户组标识符
                free_out_inode.i_flags = 3;                      // 设置文件打开方式为读写

                // 根据创建的文件类型判断是否要申请数据块，仅仅考虑两种情况，目录文件和普通文件
                // 如果是目录文件，则需要申请数据块，如果是普通文件，则不需要申请数据块
                if (type == DIR_FILE) 
                {   // 找到第一个非空闲的数据块
                    __u16 free_block = get_one_free_block_bitmap();
                    // 设置其为被占用
                    set_one_bit_of_block_bitmap(free_block + 1, BLOCK_INDEX_IN_USE, 0);
                    free_out_inode.i_blocks = 1;
                    free_out_inode.i_block[0] = free_block - FIRST_DATA_BLOCK;
                    free_out_inode.i_size = 2;
                    free_out_inode.i_links_count = 2;
                
                    // 向新建目录的数据块中填充两个dentry
                    struct ext2_dir_entry_2 dir1, dir2;
                    dir1.inode = free_inode;
                    dir1.rec_len = 256;             // 目录项所占空间大小为256个字节
                    dir1.file_type = DIR_FILE;      // 定义文件类型为目录文件
                    strcpy(dir1.name, ".");
                    dir1.name_len = 1;

                    dir2.inode = free_inode;
                    dir2.rec_len = 256;             // 目录项所占空间大小为256个字节
                    dir2.file_type = DIR_FILE;      // 定义文件类型为目录文件
                    strcpy(dir2.name, "..");
                    dir2.name_len = 2;

                    // 重新打开文件
                    if ((current_disk = fopen(DISK, "r+")) == NULL)
                    {
                        // 重新打开文件失败！
                        printf("重新打开文件失败！\n");
                        return IS_FALSE;
                    }

                    // 写入两个dentry
                    fseek(current_disk, HOME_LENGTH + free_block*EVERY_BLOCK, SEEK_SET);
                    fwrite(&dir1, DIR_DENTRY_LENGTH, 1, current_disk);
                    fseek(current_disk, HOME_LENGTH + free_block*EVERY_BLOCK + DIR_DENTRY_LENGTH, SEEK_SET);
                    fwrite(&dir2, DIR_DENTRY_LENGTH, 1, current_disk);
                } else 
                {
                    printf("this is not dir file, it is ordinary file\n");
                    free_out_inode.i_blocks = 0;
                    free_out_inode.i_size = 0;
                    free_out_inode.i_links_count = 0;
                }
                printf("free_inode = %d\n", free_inode);
                fclose(current_disk);
                if ((current_disk = fopen(DISK, "r+")) == NULL)
                {   
                    printf("重新打开文件失败！\n");
                    return IS_FALSE;
                }
                // 将新的inode节点写入
                if (0 == fseek(current_disk, HOME_LENGTH + FIRST_DATA_BLOCK*EVERY_BLOCK + OUT_INODE_LENGTH*(free_inode - 1), SEEK_SET))
                {
                    printf("xindedinodddddddddddddddd\n");
                } else 
                {
                    printf("ddddddddddddddddddddddddd\n");
                }
                fwrite(&free_out_inode, OUT_INODE_LENGTH, 1, current_disk);
                fclose(current_disk);
                // 生成一个新的dentry节点
                struct ext2_dir_entry_2 new_dentry;
                new_dentry.file_type = type;        // 填充文件类型
                strcpy(new_dentry.name, filename);
                new_dentry.name_len = strlen(filename);
                new_dentry.rec_len = 256;
                new_dentry.inode = free_inode;
                // 读出数据之后，首先判断i_blocks以及i_size
                // 如果当前的i_size项数是4的倍数，说明需要申请一个数据块，如果不是，则找到其数据块
                printf("------------------%d-----------------\n", now_inode.i_size);
                printf("\n\n\n\n%s\n\n\n\n", new_dentry.name);
                if (now_inode.i_links_count % 4)
                { 
                    // 不需要申请数据块，直接将dentry写入原有的inode对应的目录项
                    // 计算目录表项当前的偏移块数， 注意其从0开始
                    __u16 offset_dentry_block = FIRST_DATA_BLOCK + now_inode.i_block[now_inode.i_blocks - 1];
                    // 计算目录表项目的偏移
                    __u16 offset_dentry_number = now_inode.i_size % 4;
                    if ((current_disk = fopen(DISK, "r+")) == NULL)
                    {
                        printf("重新打开文件失败！\n");
                        return IS_FALSE;
                    }

                    // 将新建的dentry写入磁盘
                    if (0 == fseek(current_disk, HOME_LENGTH + offset_dentry_block * EVERY_BLOCK + offset_dentry_number * DIR_DENTRY_LENGTH, SEEK_SET))
                    {
                        printf("\n\n\n\n已经找到对应的文件位置！\n");
                    } else
                    {
                        printf("\n\n\n\n找文件失败！\n");
                    }
                    fwrite(&new_dentry, DIR_DENTRY_LENGTH, 1, current_disk);
                    fclose(current_disk);
                    now_inode.i_size++;
                } else 
                {   // 需要申请数据块的情形
                    now_inode.i_size++;
                    // 申请一个空闲数据块
                    __u16 new_dentry_block = get_one_free_block_bitmap();
                    // 设置其被占用
                    set_one_bit_of_block_bitmap(new_dentry_block + 1, BLOCK_INDEX_IN_USE, 0);
                    now_inode.i_block[now_inode.i_blocks] = new_dentry_block - FIRST_DATA_BLOCK;
                    now_inode.i_blocks++;
                    printf("new_dentry_block = %d\n", new_dentry_block);
                    if ((current_disk = fopen(DISK, "r+")) == NULL)
                    {
                        printf("打开文件失败！\n");
                        return IS_FALSE;
                    }
                    printf("new_dentry_block - 1 = %d\n", new_dentry_block - 1);
                    printf("inode_size = %d\n", now_inode.i_size);
                    printf("\n\n\n%d\n\n\n", HOME_LENGTH + new_dentry_block * EVERY_BLOCK);
                    // 将新建的dentry写入磁盘
                    fseek(current_disk, HOME_LENGTH + new_dentry_block * EVERY_BLOCK, SEEK_SET);
                    fwrite(&new_dentry, DIR_DENTRY_LENGTH, 1, current_disk);
                    fclose(current_disk);
                }
                now_inode.i_links_count++;
                printf("%d\n", current_inode);
                if ((current_disk = fopen(DISK, "r+")) == NULL)
                {
                    printf("错误！\n");
                    return IS_FALSE;
                }
                if (0 == fseek(current_disk, HOME_LENGTH + FIRST_INODE_BLOCK*EVERY_BLOCK + OUT_INODE_LENGTH * (current_inode - 1), SEEK_SET))
                {
                    printf("已经找到inode节点位置！\n");
                }
                if (fwrite(&now_inode, OUT_INODE_LENGTH, 1, current_disk))
                {
                    printf("写入磁盘成功！\n");
                }
                printf("创建文件成功\n");
                fclose(current_disk);
                return IS_TRUE;
            } else 
            {
                printf("没有空闲的inode节点！\n");
            }
        } else 
        {
            printf("文件系统内部错误！\n");
        }
    }
    return IS_FALSE;
}

// 删除文件
BOOL deleteFile(char* filename, char* current)
{   // filename 为要删除的文件名称，current为当前工作目录
    __u16 current_inode = findInodeByDirFilename(current);
    if (current_inode == 0) 
    {
        printf("没有找到对应的inode节点，文件系统错误！\n");
        return IS_FALSE;
    } else 
    {   // 获取inode节点的内容
        struct ext2_out_inode now_inode;
        struct ext2_dir_entry_2 dir_dentry;
        if ((current_disk = fopen(DISK, "r+")) != NULL)
        {
            printf("current_inode = %d\n", current_inode);

            fseek(current_disk,  HOME_LENGTH + FIRST_INODE_BLOCK*EVERY_BLOCK + OUT_INODE_LENGTH * (current_inode - 1), SEEK_SET);
            fread(&now_inode, OUT_INODE_LENGTH, 1, current_disk);

            printf("i_links_count = %d\n", now_inode.i_links_count);

            // 找到其所使用的数据块
            int i;
            for (i = 0; i < now_inode.i_links_count; i++)
            {
                fseek(current_disk, HOME_LENGTH + (FIRST_DATA_BLOCK + now_inode.i_block[i/4])*EVERY_BLOCK + DIR_DENTRY_LENGTH*(i%4), SEEK_SET);
                fread(&dir_dentry, DIR_DENTRY_LENGTH, 1, current_disk);
                if (0 == strcmp(dir_dentry.name, filename))
                {
                    // 找到了指定的文件，首先判断文件的类型
                    // if (dir_dentry.file_type == DIR_FILE)
                    // {   // 如果是目录文件
                    //     fclose(current_disk);
                    //     printf("该文件的inode节点编号为：%d, 现在开始删除该目录！\n", dir_dentry.inode);
                    //     printf("当前路径为：%s%s\n", current, filename);
                    //     return IS_TRUE;
                    // }
                    // if (dir_dentry.file_type == ORDINARY_FILE)
                    // {
                        // 如果是普通文件
                        printf("该文件的inode节点编号为：%d, 现在开始删除该文件！\n", dir_dentry.inode);
                        printf("当前路径为：%s%s\n", current, filename);

                        // 通过dentry中的inode编号，讲该inode节点的位图设置为0
                        set_one_bit_of_index_bitmap(dir_dentry.inode, BLOCK_INDEX_NOT_USE, 0);
                        // 删除该dentry目录，注意一定是删除最后一项的时候有效，否则无效
                        // 修改inode节点并且写回
                        fclose(current_disk);
                        if ((current_disk = fopen(DISK, "r+")) == NULL)
                        {
                            printf("读文件失败！\n");
                            return IS_FALSE;
                        }
                        struct ext2_out_inode the_next_inode;

                        // 找到文件对应的inode的位置
                        printf("找到的inode：%d\n", HOME_LENGTH + FIRST_INODE_BLOCK*EVERY_BLOCK + OUT_INODE_LENGTH * (current_inode - 1));
                        if ( 0 == fseek(current_disk, HOME_LENGTH + FIRST_INODE_BLOCK*EVERY_BLOCK + OUT_INODE_LENGTH * (current_inode - 1), SEEK_SET))
                        {
                            printf("-----------------------------------------------------------------------\n");
                        } else 
                        {
                            printf("///////////////////////////////////////////////////////////////////////\n");
                        }
                        fread(&the_next_inode, OUT_INODE_LENGTH, 1, current_disk);

                        the_next_inode.i_links_count--;

                        fclose(current_disk);
                        if ((current_disk = fopen(DISK, "r+")) == NULL)
                        {
                            printf("读文件失败！\n");
                            return IS_FALSE;
                        }

                        printf("HOME_LENGTH + FIRST_INODE_BLOCK*EVERY_BLOCK + OUT_INODE_LENGTH * (current_inode - 1) = %d\n", HOME_LENGTH + FIRST_INODE_BLOCK*EVERY_BLOCK + OUT_INODE_LENGTH * (current_inode - 1));

                        // 将inode写回
                        if ( 0 == fseek(current_disk, HOME_LENGTH + FIRST_INODE_BLOCK*EVERY_BLOCK + OUT_INODE_LENGTH * (current_inode - 1), SEEK_SET))
                        {
                            printf("-----------------------------------------------------------------------\n");
                        } else 
                        {
                            printf("///////////////////////////////////////////////////////////////////////\n");
                        }
                        fwrite(&the_next_inode, OUT_INODE_LENGTH, 1, current_disk);
                        printf("%d^^^^^^^^^^^^^^^^^^^^^\n", the_next_inode.i_links_count);
                        fclose(current_disk);
                        return IS_TRUE;
                    // }
                }
            }
            printf("没有找到该文件！\n");
            fclose(current_disk);
        }
    }
    return IS_FALSE;
}

// 列出此目录下的所有文件
BOOL selectFile(char* current)
{   // current 为当前工作目录
    __u16 current_inode = findInodeByDirFilename(current);
    if (current_inode == 0) 
    {
        printf("没有找到对应的inode节点，文件系统错误！\n");
        return IS_FALSE;
    } else 
    {   // 获取inode节点的内容
        struct ext2_out_inode now_inode;
        struct ext2_dir_entry_2 dir_dentry;
        if ((current_disk = fopen(DISK, "r+")) != NULL)
        {
            printf("select = %d\n", HOME_LENGTH + FIRST_INODE_BLOCK*EVERY_BLOCK + OUT_INODE_LENGTH * (current_inode - 1));
            fseek(current_disk, HOME_LENGTH + FIRST_INODE_BLOCK*EVERY_BLOCK + OUT_INODE_LENGTH * (current_inode - 1), SEEK_SET);
            fread(&now_inode, OUT_INODE_LENGTH, 1, current_disk);
            int i;
            printf("current_inode = %d\n", current_inode);
            printf("now_inode.i_links_count = %d\n", now_inode.i_links_count);
            for (i = 0; i < now_inode.i_links_count; i++)
            {
                fseek(current_disk, HOME_LENGTH + (FIRST_DATA_BLOCK + now_inode.i_block[i/4])*EVERY_BLOCK + DIR_DENTRY_LENGTH * (i % 4), SEEK_SET);
                fread(&dir_dentry, DIR_DENTRY_LENGTH, 1, current_disk);
                printf("%s\n", dir_dentry.name);
            }
            fclose(current_disk);
            return IS_TRUE; 
        }
    }
    return IS_FALSE;
}

// 打开文件
BOOL openFile(char* filename, char* current)
{   // current 为当前工作目录
    __u16 current_inode = findInodeByDirFilename(current);
    if (current_inode == 0) 
    {
        printf("没有找到对应的inode节点，文件系统错误！\n");
        return IS_FALSE;
    } else 
    {   // 获取inode节点的内容
        struct ext2_out_inode now_inode;
        struct ext2_dir_entry_2 dir_dentry;
        if ((current_disk = fopen(DISK, "r+")) != NULL)
        {
            fseek(current_disk, HOME_LENGTH + FIRST_INODE_BLOCK*EVERY_BLOCK + OUT_INODE_LENGTH * (current_inode - 1), SEEK_SET);
            fread(&now_inode, OUT_INODE_LENGTH, 1, current_disk);
            int i;
            for (i = 0; i < now_inode.i_links_count; i++)
            {
                fseek(current_disk, HOME_LENGTH + (FIRST_DATA_BLOCK + now_inode.i_block[i/4])*EVERY_BLOCK + DIR_DENTRY_LENGTH*(i%4), SEEK_SET);
                fread(&dir_dentry, DIR_DENTRY_LENGTH, 1, current_disk);
                if (0 == strcmp(dir_dentry.name, filename))
                {
                    // 找到了指定的文件，首先判断文件的类型
                    if (dir_dentry.file_type == DIR_FILE)
                    {   // 如果是目录文件
                        fclose(current_disk);
                        printf("该文件的inode节点编号为：%d, 成功打开该目录！\n", dir_dentry.inode);
                        printf("当前路径为：%s%s\n", current, filename);
                        return IS_TRUE;
                    }
                    if (dir_dentry.file_type == ORDINARY_FILE)
                    {
                        // 如果是普通文件
                        printf("该文件的inode节点编号为：%d, 成功打开该文件！\n", dir_dentry.inode);
                        fclose(current_disk);
                        return IS_TRUE;
                    }
                }
            }
            fclose(current_disk);
            return IS_FALSE; 
        }
    }
    return IS_FALSE;
}

// 写文件（修改文件）
BOOL modifyFile(char* filename, char* current)
{    // current 为当前工作目录
    __u16 current_inode = findInodeByDirFilename(current);
    if (current_inode == 0) 
    {
        printf("没有找到对应的inode节点，文件系统错误！\n");
        return IS_FALSE;
    } else 
    {   // 获取inode节点的内容
        struct ext2_out_inode now_inode;
        struct ext2_dir_entry_2 dir_dentry;
        if ((current_disk = fopen(DISK, "r+")) != NULL)
        {
            fseek(current_disk, HOME_LENGTH + FIRST_INODE_BLOCK*EVERY_BLOCK + OUT_INODE_LENGTH * (current_inode - 1), SEEK_SET);
            fread(&now_inode, OUT_INODE_LENGTH, 1, current_disk);
            int i;
            for (i = 0; i < now_inode.i_links_count; i++)
            {
                fseek(current_disk, HOME_LENGTH + (FIRST_DATA_BLOCK + now_inode.i_block[i/4])*EVERY_BLOCK + DIR_DENTRY_LENGTH*(i%4), SEEK_SET);
                fread(&dir_dentry, DIR_DENTRY_LENGTH, 1, current_disk);
                if (0 == strcmp(dir_dentry.name, filename))
                {
                    // 找到了指定的文件，首先判断文件的类型
                    if (dir_dentry.file_type == DIR_FILE)
                    {
                        fclose(current_disk);
                        printf("由于文件是目录文件，不能执行修改操作！\n");
                        return IS_TRUE;
                    }
                    if (dir_dentry.file_type == ORDINARY_FILE)
                    {
                        fclose(current_disk);
                        // 如果是普通文件
                        printf("the inode number is %d\n", dir_dentry.inode);
                        printf("please input your words, ending by Enter:\n");
                        char buffer[1024];
                        getchar();
                        gets(buffer);
                        puts(buffer);                    
                        // 申请一个空闲数据块
                        __u16 new_dentry_block = get_one_free_block_bitmap() + 1;
                        // 设置其被占用
                        set_one_bit_of_block_bitmap(new_dentry_block + 1, BLOCK_INDEX_IN_USE, 0);
                        // printf("new_dentry_block = %d\n", new_dentry_block);
                        if ((current_disk = fopen(DISK, "r+")) == NULL)
                        {
                            printf("写入文件失败！\n");
                            return IS_FALSE;
                        }
                        // printf("HOME_LENGTH + new_dentry_block * EVERY_BLOCK = %d\n", HOME_LENGTH + new_dentry_block * EVERY_BLOCK);
                        fseek(current_disk, HOME_LENGTH + new_dentry_block * EVERY_BLOCK, SEEK_SET);
                        fwrite(&buffer, 1024, 1, current_disk);
                        fclose(current_disk);

                        // now_inode.i_block[now_inode.i_blocks] = new_dentry_block - FIRST_DATA_BLOCK;
                        // now_inode.i_blocks++;

                        if ((current_disk = fopen(DISK, "r+")) == NULL)
                        {
                            printf("写入文件失败！\n");
                            return IS_FALSE;
                        }

                        struct ext2_out_inode inode_rewrite;

                        // 找到该文件对应的inode
                        fseek(current_disk, HOME_LENGTH + FIRST_INODE_BLOCK*EVERY_BLOCK + OUT_INODE_LENGTH * (dir_dentry.inode - 1), SEEK_SET);
                        fread(&inode_rewrite, OUT_INODE_LENGTH, 1, current_disk);
                        fclose(current_disk);

                        if ((current_disk = fopen(DISK, "r+")) == NULL)
                        {
                            printf("写入文件失败！\n");
                            return IS_FALSE;
                        }

                        inode_rewrite.i_block[inode_rewrite.i_blocks] = new_dentry_block - FIRST_DATA_BLOCK;
                        inode_rewrite.i_blocks++;

                        // printf("对应文件的inode：%d\n", HOME_LENGTH + FIRST_INODE_BLOCK*EVERY_BLOCK + OUT_INODE_LENGTH * (dir_dentry.inode - 1));
                        // printf("\n%d\n", inode_rewrite.i_block[0]);

                        // 回写inode
                        fseek(current_disk, HOME_LENGTH + FIRST_INODE_BLOCK*EVERY_BLOCK + OUT_INODE_LENGTH * (dir_dentry.inode - 1), SEEK_SET);
                        fwrite(&inode_rewrite, OUT_INODE_LENGTH, 1, current_disk);

                        printf("该文件的inode节点编号为：%d, 成功写入该文件！\n", dir_dentry.inode);
                        fclose(current_disk);
                        return IS_TRUE;
                    }
                }
            }
            fclose(current_disk);
            return IS_FALSE; 
        
        }
    }
}

// 读文件（将文件读出）
BOOL readFile(char* filename, char* current)
{   // current 为当前工作目录
    __u16 current_inode = findInodeByDirFilename(current);
    if (current_inode == 0) 
    {
        printf("没有找到对应的inode节点，文件系统错误！\n");
        return IS_FALSE;
    } else 
    {   // 获取inode节点的内容
        struct ext2_out_inode now_inode;
        struct ext2_dir_entry_2 dir_dentry;
        if ((current_disk = fopen(DISK, "r+")) != NULL)
        {
            fseek(current_disk, HOME_LENGTH + FIRST_INODE_BLOCK*EVERY_BLOCK + OUT_INODE_LENGTH * (current_inode - 1), SEEK_SET);
            fread(&now_inode, OUT_INODE_LENGTH, 1, current_disk);
            int i;
            for (i = 0; i < now_inode.i_links_count; i++)
            {
                fseek(current_disk, HOME_LENGTH + (FIRST_DATA_BLOCK + now_inode.i_block[i/4])*EVERY_BLOCK + DIR_DENTRY_LENGTH*(i%4), SEEK_SET);
                fread(&dir_dentry, DIR_DENTRY_LENGTH, 1, current_disk);
                if (0 == strcmp(dir_dentry.name, filename))
                {
                    // 找到了指定的文件，首先判断文件的类型
                    if (dir_dentry.file_type == DIR_FILE)
                    {
                        fclose(current_disk);
                        printf("由于文件是目录文件，不能执行读操作，请使用列出所有文件的功能！\n");
                        return IS_TRUE;
                    }
                    if (dir_dentry.file_type == ORDINARY_FILE)
                    {
                        fclose(current_disk);
                        // 如果是普通文件
                        printf("the inode number is %d\n", dir_dentry.inode);
                        printf("this is the content of the file:\n");
                        char buffer[1024];
                        if ((current_disk = fopen(DISK, "r+")) == NULL)
                        {
                            printf("读文件失败！\n");
                            return IS_FALSE;
                        }
                        struct ext2_out_inode the_next_inode;

                        // 找到文件对应的inode的位置
                        // printf("找到的inode：%d\n", HOME_LENGTH + FIRST_INODE_BLOCK*EVERY_BLOCK + OUT_INODE_LENGTH * (dir_dentry.inode - 1));
                        if ( 0 == fseek(current_disk, HOME_LENGTH + FIRST_INODE_BLOCK*EVERY_BLOCK + OUT_INODE_LENGTH * (dir_dentry.inode - 1), SEEK_SET))
                        {
                            printf("-----------------------------------------------------------------------\n");
                        } else 
                        {
                            printf("///////////////////////////////////////////////////////////////////////\n");
                        }
                        fread(&the_next_inode, OUT_INODE_LENGTH, 1, current_disk);
                        fclose(current_disk);
                        // printf("the_next_inode = %d\n", the_next_inode.i_block[0]);

                        if ((current_disk = fopen(DISK, "r+")) == NULL)
                        {
                            printf("读文件失败！\n");
                            return IS_FALSE;
                        }
                        // 找到inode对应的数据块的位置
                        // printf("HOME_LENGTH + (FIRST_DATA_BLOCK + the_next_inode.i_block[0])*EVERY_BLOCK = %d\n", HOME_LENGTH + (FIRST_DATA_BLOCK + the_next_inode.i_block[0])*EVERY_BLOCK);
                        fseek(current_disk, HOME_LENGTH + (FIRST_DATA_BLOCK + the_next_inode.i_block[0] - 1)*EVERY_BLOCK, SEEK_SET);
                        fread(&buffer, 1024, 1, current_disk);
                        puts(buffer);
                        printf("-----------------------------------------------------------------------\n");
                        fclose(current_disk);
                        printf("该文件的inode节点编号为：%d, 成功读出该文件！\n", dir_dentry.inode);
                        return IS_TRUE;
                    }
                }
            }

            printf("没有找到该文件，请检查后重试！\n");

            fclose(current_disk);
            return IS_FALSE; 
        }
    }
    printf("文件系统操作错误哈哈哈哈！\n");
    return IS_FALSE;
}

// 改变当前工作目录
BOOL changeFile(char* current, char* filename)
{   // current 为当前工作目录
    __u16 current_inode = findInodeByDirFilename(current);
    if (current_inode == 0) 
    {
        printf("没有找到对应的inode节点，文件系统错误！\n");
        return IS_FALSE;
    } else 
    {   // 获取inode节点的内容
        struct ext2_out_inode now_inode;
        struct ext2_dir_entry_2 dir_dentry;
        if ((current_disk = fopen(DISK, "r+")) != NULL)
        {
            // printf("select = %d\n", HOME_LENGTH + FIRST_INODE_BLOCK*EVERY_BLOCK + OUT_INODE_LENGTH * (current_inode - 1));
            fseek(current_disk, HOME_LENGTH + FIRST_INODE_BLOCK*EVERY_BLOCK + OUT_INODE_LENGTH * (current_inode - 1), SEEK_SET);
            fread(&now_inode, OUT_INODE_LENGTH, 1, current_disk);
            int i;
            // printf("current_inode = %d\n", current_inode);
            // printf("now_inode.i_links_count = %d\n", now_inode.i_links_count);
            for (i = 0; i < now_inode.i_links_count; i++)
            {
                fseek(current_disk, HOME_LENGTH + (FIRST_DATA_BLOCK + now_inode.i_block[i/4])*EVERY_BLOCK + DIR_DENTRY_LENGTH * (i % 4), SEEK_SET);
                fread(&dir_dentry, DIR_DENTRY_LENGTH, 1, current_disk);
                // printf("%s\n", dir_dentry.name);
                if (dir_dentry.file_type == DIR_FILE && (0 == strcmp(filename, "..")))
                {
                    int j;
                    printf("%s, %d\n", current, strlen(current));
                    for (j = strlen(current) - 1; j--; )
                    {
                        if (current[j] != '/')
                        {
                            current[j] = 0;
                        } else 
                        {
                            break;
                        }
                    }
                    fclose(current_disk);
                    return IS_TRUE;
                } else if (dir_dentry.file_type == DIR_FILE && (0 == strcmp(filename, ".")))
                {
                    printf("_+++++++++++++++++++++++++++++++++++__\n");
                    fclose(current_disk);
                    return IS_TRUE;
                } else if ((dir_dentry.file_type == DIR_FILE) && (0 == strcmp(dir_dentry.name, filename)))
                {
                    if (current[strlen(current)-1] != '/') strcat(current, "/");
                    strcat(current, filename);
                    fclose(current_disk);
                    return IS_TRUE;
                } else
                {
                    continue;
                    fclose(current_disk);
                    return IS_FALSE;
                }
            }
            printf("没有找到文件或者文件不是目录文件！\n");
            return IS_FALSE; 
        }
    }
    return IS_FALSE;
}
