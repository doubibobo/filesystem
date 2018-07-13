#ifndef __DENTRY_H__
#define __DENTRY_H__

#include "exit2init.h"

// 根据目录文件的文件名，找到对应的inode节点
__u16 findInodeByDirFilename(char filename[]);

#endif