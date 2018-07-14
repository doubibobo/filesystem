#include "print.h"

void print(char* current)
{
    for (; ;)
    {
        printf("[root@root-PC]:%s\n", current);
        char command[10];
        char list[10];
        scanf("%s", command);
        // printf("%s\n\n", command);
        // printf("%s\n\n", list);
        printf("13456489497\n");
        if (0 == strcmp(command, MKDIR))
        {   
            scanf("%s", list);
            createFile(list, DIR_FILE, current);
            continue;
        }
        if (0 == strcmp(command, TOUCH))
        {
            scanf("%s", list);
            createFile(list, ORDINARY_FILE, current);
            continue;
        }
        if (0 == strcmp(command, LS))
        {
            selectFile(current);
            continue;
        }
        if (0 == strcmp(command, CD))
        {
            scanf("%s", list);
            changeFile(current, list);
            continue;
        }
        if (0 == strcmp(command, RM))
        {
            scanf("%s", list);
            deleteFile(list, current);
            continue;
        }
        if (0 == strcmp(command, CAT))
        {
            scanf("%s", list);
            readFile(list, current);
            continue;
        }
        if (0 == strcmp(command, ECHO))
        {
            scanf("%s", list);
            modifyFile(list, current);
            continue;
        }
        if (0 == strcmp(command, OPEN))
        {
            scanf("%s", list);
            openFile(list, current);
            continue;
        }
        if (0 == strcmp(command, EXIT))
        {
            printf("退出系统！\n");
            break;
        } 
        if (0 == strcmp(command, CLEAR))
        {
            clean();
            continue;
        }
        printf("输入参数错误！\n");
    }
}

void clean()
{
    system("clear");
}