#include "myfs_fuse.h"
#include "my.h"
#include "syscall.h"
#include <asm-generic/errno.h>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <fuse3/fuse.h>

static char pathOfFS[] = "/home/emon100/myfs.hd";

int fs_readdir(const char *path,void *buf, fuse_fill_dir_t filler,
        off_t off,struct fuse_file_info *fi,
        enum fuse_readdir_flags flags){

    int r = chdir(0,path);
    if(r<0) return -ENOTDIR;

    DirectoryEntry *entries = getDirectory(INumber2INode(r)->diskBlockId)->Entry;

    for(int i=0;i<BLOCK_SIZE/(int)sizeof(DirectoryEntry);++i){
        if(entries[i].inumber!=-1){
            filler(buf,entries[i].name,NULL,0,FUSE_FILL_DIR_PLUS);
        }
    }
    return 0;
}

int fs_getattr(const char *path, struct stat *buf, struct fuse_file_info *fi){
    memset(buf,0,sizeof(struct stat));

    int r = inumber_of_path(getFSInfo()->root_inumber,path);
    if(r<0) return -ENOENT;

    INode *i = INumber2INode(r);
    if(i){
        buf->st_size = i->filelen;
        buf->st_mode= (i->type?S_IFDIR:S_IFREG)|0777;

        buf->st_nlink=2;
        buf->st_ctime = i->createTime;
        buf->st_blksize = BLOCK_SIZE;
        buf->st_blocks = i->diskBlockCount;
        return 0;
    }else{
        return -ENOENT;
    }
}

int fs_read(const char *path, char *buf, size_t size, off_t off, struct fuse_file_info *fi){

    int r = inumber_of_path(getFSInfo()->root_inumber,path);
    if(r<0) return -ENOENT;

    INode *i = INumber2INode(r);
    if(i&&i->type!=1){
        return read(r,off,buf,size);
    }else{
        return -EISDIR;
    }
}

int fs_write(const char *path,const char *buf, size_t size, off_t off, struct fuse_file_info *fi){
    int r = inumber_of_path(getFSInfo()->root_inumber,path);
    if(r<0) return -ENOENT;

    INode *i = INumber2INode(r);
    if(i&&i->type!=1){
        r = write(r,off,(void *)buf,size);
        return r;
    }else{
        return -EISDIR;
    }
}

int fs_rmdir(const char *path){
    int r = inumber_of_path(getFSInfo()->root_inumber,path);
    INode *inode= INumber2INode(r);
    if(inode==NULL){
        return -ENOENT;
    }else if(r==0){
        return -EACCES;
    }else if(inode->type==0){
        return -ENOTDIR;
    }
    Directory *dir = getDirectory(inode->diskBlockId);
    DirectoryEntry *d = dir->Entry;
    int count=0;
    INUMBER upper=-1;
    for(int i=0;i<sizeof(Directory)/sizeof(DirectoryEntry);++i){
        if(d[i].inumber!=-1){
            ++count;
            if(strcmp(d[i].name,"..")==0){
                upper = d[i].inumber;
            }
        }
    }
    INode *upperdir = INumber2INode(upper);
    if(count>2){
        return -ENOTEMPTY;
    }else if(upperdir==NULL){
        return -ENOENT;
    }else if(upperdir->type==0){
        return -ENOTDIR;
    }else{
        Directory *upperDirectory = getDirectory(upperdir->diskBlockId);
        DirectoryEntry *removingEntry = find_entry_in_directory_by_INUMBER(upperDirectory,r);
        dealloc_inode(removingEntry->inumber);
        removingEntry->inumber=-1;
        return 0;
    }
}

int fs_mkdir(const char *path, mode_t mode){
    int pos = -1;
    for(const char *p = path;*p;++p){
        if(*p=='/'){
            pos = p-path;
        }
    }
    if(pos==-1||pos==0){
        INUMBER upper = inumber_of_path(getFSInfo()->root_inumber,path);
        return mkdir(upper,path);
    }else{
        char buf[5000]={};
        strncpy(buf,path,5000);
        buf[pos]='\0';
        INUMBER upper = inumber_of_path(getFSInfo()->root_inumber,buf);
        return mkdir(upper,buf+pos+1);
    }
}

int fs_create(const char *path, mode_t mode, struct fuse_file_info *fi){
    int pos = -1;
    for(const char *p = path;*p;++p){
        if(*p=='/'){
            pos = p-path;
        }
    }

    if(pos==-1||pos==0){
        INUMBER upper = inumber_of_path(getFSInfo()->root_inumber,path);
        INUMBER inumber = alloc_empty_inode(
                0,
                0,
                0,
                0,
                0777,
                0
        );

        Directory *rootDir = getDirectory(INumber2INode(getFSInfo()->root_inumber)->diskBlockId);
        path = path + pos + 1;
        int err = add_directory_entry(rootDir, path, inumber);
        if(err<0){
            dealloc_inode(inumber);
            return err;
        }
        printDirectory(rootDir);
        return 0;
    }else{
        char buf[5000]={};
        strncpy(buf,path,5000);
        buf[pos]='\0';
        INUMBER upper = inumber_of_path(getFSInfo()->root_inumber,buf);
        if(upper==-1){
            return -1;
        }
        INUMBER inumber = alloc_empty_inode(
                0,
                0,
                0,
                0,
                0777,
                0
        );
        INode *dir = INumber2INode(inumber);
        if(dir==NULL||dir->type!=1){
            dealloc_inode(inumber);
            return -1;
        }

        Directory *rootDir = getDirectory(dir->diskBlockId);

        int err = add_directory_entry(rootDir, path, inumber);
        if(err<0){
            dealloc_inode(inumber);
            return -1;
        }
        return 0;
    }
}
int fs_unlink(const char *path){
    return rm(getFSInfo()->root_inumber,path+1);
}
