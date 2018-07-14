#include "dentry.h"
#include "exit2init.h"
#include "foperator.h"
#include "memory.h"

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

    printf("%ld\n", sizeof(unsigned short));
    printf("%ld\n", sizeof(unsigned long));

    char test[10] = "/";
    findInodeByDirFilename(test);

    selectFile(test);

    printf("********\n");
    printf("********\n");
    printf("********\n");

    if (createFile("var456", DIR_FILE, test))
    {
        printf("-----------------------------------\n");
        selectFile(test);
    }

    if (createFile("dir2", ORDINARY_FILE, test))
    {
        printf("+++++++++++++++++++++++++++++++++++\n");
        selectFile(test);
    }

    if (createFile("dir3", ORDINARY_FILE, test))
    {
        printf("+++++++++++++++++++++++++++++++++++\n");
        selectFile(test);
    }

    printf("********\n");
    printf("********\n");
    printf("********\n");

    if (openFile("var456", test))
    {
        printf("temp = %s\n", test);
        selectFile(test);
    }

    if (openFile("dir2", test))
    {
        printf("temp = %s\n", test);
        selectFile(test);
    }

    selectFile(test);

    printf("现在开始写文件！\n");
    if (modifyFile("dir3", test))
    {
        printf("写文件结束\n");
        // selectFile(test);
    }

    selectFile(test);

    if (readFile("dir3", test))
    {
        printf("+++++++++++++++++++++++++++++++++++\n");
        // selectFile(test);
    }

    selectFile(test);
    
    printf("开始删除文件！\n");

    if (deleteFile("dir3", test))
    {
        selectFile(test);
    }

    // if ()
    // {
    //     printf("***********************************\n");
    //     printf("~~~~~~~~~~~~~~~~%s~~~~~~~~~~~~~~~~~\n", test);
    //     selectFile(test);
    // }
    


    // if (createFile("dir", DIR_FILE, test))
    // {
    //     printf("+++++++++++++++++++++++++++++++++++\n");
    //     selectFile(test);
    // }

    // super_block_out_to_in();

    return 0;
}