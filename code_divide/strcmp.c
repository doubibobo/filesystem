#include <stdio.h>
#include <string.h>

struct Student
{   
    char name[10];
};

int main()
{
    struct Student student;
    strcpy(student.name, "doubibobo");
    char* str1 = "doubibobo";
    if (0 == strcmp(student.name, str1))
    {
        printf("str1 = str2\n");
    } else 
    {
        printf("str2 != str2\n");
    }
    return 0;
}