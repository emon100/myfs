#ifndef SYSCALL_H
#define SYSCALL_H
#include "my.h"

void ls(Directory *d);

int32_t rm(Directory *d,const char *path);//删除文件

int32_t rmdir(Directory *d, const char *path);//删除目录

INUMBER open(const char *path);//打开文件

int64_t write(INUMBER fd, int64_t offset, void *buf, size_t count);//写入
int64_t read(INUMBER fd, int64_t offset, void *buf, size_t count);//读取

int64_t calculate_capacity(INode *ino);//计算文件可容纳的内容大小


#endif // SYSCALL_H
