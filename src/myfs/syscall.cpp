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
        int err = give_file_an_empty_block(ino);
        maxWriteCount = calculate_capacity(ino) - offset;
        if(err==-1){
            break;
        }
    }
    if(maxWriteCount > (long long)count){
        //TODO: now can only write to 1 block. Improve it.
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



int32_t mkdir(INUMBER upper, const char *name)
{
    INUMBER B_Directory = make_directory(upper);
    Directory *d =getDirectory(INumber2INode(upper)->diskBlockId);
    return add_directory_entry(d,name,B_Directory);
}

vector<string> SplitString(const string& s, const string& c)
{
    vector<string> v;
    string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while(string::npos != pos2)
    {
        v.push_back(s.substr(pos1, pos2-pos1));

        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);
    }
    if(pos1 != s.length())
        v.push_back(s.substr(pos1));
    return v;
}

INUMBER chdir(INUMBER current,string path){
    INode *node = INumber2INode(current);
    if(node==NULL||node->type!=1){
        return -1;
    }
    vector<string> pathlist = SplitString(path, "/");
    Directory *no = pathlist[0].empty()?
                getDirectory(INumber2INode(getFSInfo()->root_inumber)->diskBlockId)
                :
                getDirectory(node->diskBlockCount);

    INUMBER tmp = -1;
    for(int i=1;i<(long long)pathlist.size();i++){
        tmp = find_in_directory(no,pathlist[i].c_str());
        node = INumber2INode(tmp);
        if(node&&node->type==1){
            no = getDirectory(node->diskBlockId);
        }else{
            return -1;
        }
    }
    return tmp;
}
