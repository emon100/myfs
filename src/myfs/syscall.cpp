#include <cstdio>
#include <cstring>
#include "syscall.h"

void ls(Directory *dir){
    DirectoryEntry * entries = dir->Entry;
    for(int i=0;i<BLOCK_SIZE/(int)sizeof(DirectoryEntry);++i){
        if(entries[i].inumber!=-1){
            printf("%s\t",entries[i].name);
        }
    }
}


int32_t __rm(Directory *dir, const char * p, int32_t type){
    if(dir==NULL||p==NULL){
        return -1;
    }

    char buf[5000]={};
    int i=0;

    INUMBER target = -1;
    INode *targetinode = NULL;
    while(*p!='\0'){
        if(*p!='/'){
            buf[i] = *p;
            ++i;
        }else{
            if(*(p+1)=='\0'){
                break;
            }
            i=0;
            targetinode = find_inode_in_directory(dir,buf);
            if(targetinode==NULL||targetinode->type==0){
                return -1;
            }else{
                dir = getDirectory(targetinode->diskBlockId);
            }
        }
        ++p;
    }
    target = find_in_directory(dir,buf);
    targetinode = INumber2INode(target);
    if(targetinode==NULL||targetinode->type!=type){
        return -1;
    }else{
        dealloc_inode(target);
        for(int i=0;i<(long long)(sizeof(Directory)/sizeof(DirectoryEntry));++i){
            if(dir->Entry[i].inumber==target){
                dir->Entry[i].inumber=-1;
                break;
            }
        }
        return 0;
    }
}

int32_t rmdir(Directory *dir,const char *p){
    if(p[0]=='/'){
        dir = getDirectory(INumber2INode(getFSInfo()->root_inumber)->diskBlockId);
        ++p;
    }
    return __rm(dir,p,1);
}
int32_t rm(Directory *dir,const char *p){
    if(p[0]=='/'){
        dir = getDirectory(INumber2INode(getFSInfo()->root_inumber)->diskBlockId);
        ++p;
    }
    return __rm(dir,p,0);
}

int64_t read(INUMBER fd, int64_t offset, void *buf, size_t count){
    INode *ino = INumber2INode(fd);
    if(buf==NULL||ino==NULL){
        return -1;
    }
    int64_t maxReadCount = ino->filelen - offset;
    if(maxReadCount<0) return 0;
    else if(maxReadCount > (long long)count){
        //do 1 block
        char *src =  getBlock(ino->diskBlockId);
        memcpy(buf,src+offset,count);
        return count;
    }else{
        char *src =  getBlock(ino->diskBlockId);
        memcpy(buf,src+offset,maxReadCount);
        return maxReadCount;
    }
}

int64_t write(INUMBER fd, int64_t offset, void *buf, size_t count){
    INode *ino = INumber2INode(fd);
    if(buf==NULL||ino==NULL){
        return -1;
    }
    int64_t maxWriteCount = calculate_capacity(ino) - offset;
    while(maxWriteCount<=0){
        give_file_an_empty_block(ino);
        maxWriteCount = calculate_capacity(ino) - offset;
    }
    if(maxWriteCount > (long long)count){
        //do 1 block
        char *src =  getBlock(ino->diskBlockId);
        memcpy(src+offset,buf,count);
        ino->filelen = count;
        return count;
    }else{
        char *src =  getBlock(ino->diskBlockId);
        memcpy(src+offset,buf,maxWriteCount);
        ino->filelen = maxWriteCount;
        return maxWriteCount;
    }
}

int64_t calculate_capacity(INode *inode){
    if(inode==NULL){
        return -1;
    }

    int n = inode->diskBlockCount;
    int layer =  (n - 1 + 1022)/1023;
    return (n-layer) * BLOCK_SIZE;
}