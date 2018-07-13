#include "dentry.h"

char delims[] = "/"; 

// 根据目录文件的文件名，找到对应的inode节点，filename必须以根节点开始
__u16 findInodeByDirFilename(char filename[])
{
    FILE* file_disk;
    __u16 final_inode;
    if ((file_disk = fopen(DISK, "r+")) != NULL)
    {
        // 将filename按照'/'进行分割，同时找寻节点，从1号inode节点出发
        fseek(file_disk, FIRST_INODE_BLOCK*EVERY_BLOCK + OUT_INODE_LENGTH, SEEK_SET);
        struct ext2_out_inode root_disk;   
        struct ext2_dir_entry_2 dir_dentry;
        __u8 flag = 0;
        if (fread(&root_disk, OUT_INODE_LENGTH, 1, file_disk))
        {
            // 以'/'分割字符
            char *result = strtok(filename, delims);
            while(result != NULL) { 
                int i;
                for (i = 0; i < root_disk.i_size; i++)
                {
                    fseek(file_disk, (FIRST_DATA_BLOCK + root_disk.i_block[i/4])*EVERY_BLOCK + DIR_DENTRY_LENGTH*i, SEEK_SET);
                    fread(&dir_dentry, DIR_DENTRY_LENGTH, 1, file_disk);
                    if (0 == strcmp(result, dir_dentry.name)) 
                    {
                        flag = 1;
                        break;
                    }
                } 
                // 根据找到的inode，重新定位数据块
                fseek(file_disk, FIRST_INODE_BLOCK*EVERY_BLOCK + OUT_INODE_LENGTH*(dir_dentry.inode - 1),SEEK_SET);
                fread(&root_disk, OUT_INODE_LENGTH, 1, file_disk);
                result = strtok(NULL, delims); 
                if (flag && !result)
                {
                    // 注意，返回的inode节点实际是从1开始编号的结果，如根目录返回2，root目录返回3
                    printf("已经找到对应的inode节点！\n");
                    fclose(file_disk);
                    return dir_dentry.inode;
                } else 
                {
                    flag = 0;
                }
            }
        }
        fclose(file_disk);
        return 0;
    }
    return 0;
}