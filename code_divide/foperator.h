#ifndef __FOPERATOR_H__
#define __FOPERATOR_H__

#include "dentry.h"
#include "exit2init.h"

// 创建文件
BOOL createFile(char* filename, __u8 type, char* current);

// 删除文件
BOOL deleteFile(char* filename, char* current);

// 列出此目录下的所有文件
BOOL selectFile(char* filename, char* current);

// 打开文件
BOOL openFile(char* filename, char* current);

// 写文件（修改文件）
BOOL modifyFile(char* filename, char* current);

#endif