#ifndef MYFS_FUSE_H
#define MYFS_FUSE_H
#include "my.h"
#include "syscall.h"
#define USE_FUSE_VERSION 31
#include "fuse3/fuse.h"

int fs_getattr(const char *path, struct stat *buf, struct fuse_file_info *fi);
int fs_readdir(const char *path,void *buf, fuse_fill_dir_t filler,
        off_t off,struct fuse_file_info *fi,
        enum fuse_readdir_flags flags);
int fs_read(const char *path, char *buf, size_t size, off_t off, struct fuse_file_info *fi);
int fs_write(const char *path,const  char *buf, size_t size, off_t off, struct fuse_file_info *fi);
int fs_open(const char *path, struct fuse_file_info *fi);
int fs_rmdir(const char *path);
int fs_mkdir(const char *path,mode_t mode);
int fs_create(const char *path, mode_t mode, struct fuse_file_info *fi);
int fs_unlink(const char *path);

int fs_flush(const char *path, struct fuse_file_info *fi);
#endif
