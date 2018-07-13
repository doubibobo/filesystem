#include <stdio.h>
#include <stdlib.h>  
#include <string.h>
 
int main()
{
    char str[] = "/home/doubibobo/ext2/ext"; 
    char delims[] = "/"; 
    char *result = strtok(str, delims);
    while(result != NULL) { 
        printf("result is \"%s\"\n", result); 
        result = strtok( NULL, delims); 
    }     
    printf("%s\n", str);
    return 0;
}