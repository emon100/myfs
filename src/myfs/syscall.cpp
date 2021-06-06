#include <cstring>
#include "syscall.h"

int32_t __rm(INUMBER dir, const char * p, int32_t type){
    INode *dirInode = INumber2INode(dir);
    if(dirInode==NULL||p==NULL||dirInode->type==0){
        return -1;
    }

    char buf[5000]={};
    int i=0;
    
    INUMBER target = inumber_of_path(dir,p);
    INode *targetinode = INumber2INode(target);

    if(targetinode==NULL||targetinode->type!=type){
        return -1;
    }else{
        --(targetinode->qcount);
        if(targetinode->qcount<=0){
            dealloc_inode(target);
        }
        Directory *directory = getDirectory(dirInode->diskBlockId);
        for(int i=0;i<(long long)(sizeof(Directory)/sizeof(DirectoryEntry));++i){
            if(directory->Entry[i].inumber==target){
                directory->Entry[i].inumber=-1;
                break;
            }
        }
        return 0;
    }
}

int32_t rmdir(INUMBER dir,const char *p){
    if(p[0]=='/'){
        dir = getFSInfo()->root_inumber;
        ++p;
    }
    return __rm(dir,p,1);
}
int32_t rm(INUMBER dir,const char *p){
    if(p[0]=='/'){
        dir = getFSInfo()->root_inumber;
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

int64_t write(INUMBER fd, int64_t offset, void *buf, size_t count){//only overwrite now
    INode *ino = INumber2INode(fd);
    if(buf==NULL||ino==NULL){
        return -1;
    }
    if(offset+count>_4KB){
        //TODO: now can only write to 1 block. Improve it.
        return -1;
    }
    int64_t maxWriteCount = calculate_capacity(ino) - offset;
    while(maxWriteCount<=0){
        int err = give_file_an_empty_block(ino);
        maxWriteCount = calculate_capacity(ino) - offset;
        if(err==-1){
            break;
        }
    }
    if(maxWriteCount > (long long)count){
        char *src =  getBlock(ino->diskBlockId);
        memcpy(src+offset,buf,count);
        ino->filelen = offset + count;
        return count;
    }else{
        char *src =  getBlock(ino->diskBlockId);
        memcpy(src+offset,buf,maxWriteCount);
        ino->filelen = offset + maxWriteCount;
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



int32_t mkdir(INUMBER upper, const char *name)
{
    if(upper<0||INumber2INode(upper)->type!=1){
        return -1;
    }
    INUMBER B_Directory = make_directory(upper);
    Directory *d =getDirectory(INumber2INode(upper)->diskBlockId);
    return add_directory_entry(d,name,B_Directory);
}

INUMBER inumber_of_path(INUMBER current, const char * path){
    if(path==NULL){
        return -1;
    }
    INode *currentINode;
    if(path[0]=='/'){
        current = getFSInfo()->root_inumber;
        path+=1;
    }
    currentINode = INumber2INode(current);
    if(currentINode==NULL||currentINode->type==0){
        return -1;
    }

    char buf[501]={};
    int i=0;
    Directory *dir = getDirectory(currentINode->diskBlockId);
    while(*path!='\0'&&dir!=NULL){
        if(*path!='/'){
            if(i==500){
                return -1;
            }
            buf[i] = *path;
            ++i;
        }else{
            if(*(path+1)=='\0'){
                current = find_in_directory(dir,buf);
                currentINode = INumber2INode(current);
                if(currentINode&&currentINode->type==1){
                    return current;//finding directory and get directory;
                }else{
                    return -1;
                }
            }
            buf[i]='\0';
            i=0;
            current = find_in_directory(dir,buf);
            currentINode = INumber2INode(current);
            if(currentINode==NULL){
                return -1;
            }else if(currentINode->type==0){
                dir = NULL;
            }else{
                dir = getDirectory(currentINode->diskBlockId);
            }
        }
        ++path;
    }
    if(i!=0){
        current = find_in_directory(dir,buf);
    }
    return current;
}

INUMBER chdir(INUMBER current,const char *path){
    if(path==NULL){
        return -1;
    }
    INUMBER tmp =  inumber_of_path(current,path);
    INode *tnode = INumber2INode(tmp);
    if(tnode&&tnode->type==1){
        return tmp;
    }else{
        return -1;
    }
}
