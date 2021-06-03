#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <ctime>
#include "my.h"


static char *buf;
char *getRawFs(){
    return buf;
}

void setRawFs(char *b){
    buf = b;
}

void printFSInfo(FSInfo *fs){
    printf("----FSINFO-----\n");
    printf("fs_size :%ld\n",fs->fs_size);
    printf("block_size :%ld\n",fs->block_size);
    printf("file_num :%ld\n",fs->inode_num);
    printf("empty_block_num :%ld\n",fs->empty_block_num);
    printf("root_inumber :%ld\n",fs->root_inumber);
    printf("----FSINFO--END-----\n");
}

void printINodeInfo(INode *i){
    printf("----INodeINFO-----\n");
    printf("id :%d\n",i->id);
    printf("type :%s\n",i->type?"directory":"file");
    printf("sfd :%d\n",i->sfd_id);
    printf("filelen :%d\n",i->filelen);
    printf("diskBlockNum :%d\n",i->diskBlockNum);
    printf("diskBlockId :%d\n",i->diskBlockId);
    printf("qcount :%d\n",i->qcount);
    printf("time :%u\n",i->createTime);
    printf("----INodeINFO--END-----\n");
}

void display_directory(Directory *dir){
    DirectoryEntry * entries = dir->Entry;

    printf("----Directory INFO-----\n");
    printf("name\t\tINumber");
    int count=0;
    for(int i=0;i<BLOCK_SIZE/(int)sizeof(DirectoryEntry);++i){
        if(entries[i].inumber!=-1){
            printf("%s\t\t%l",entries[i].name,entries[i].inumber);
            ++count;
        }
    }
    printf("Total files: %d",count);
    printf("----INodeINFO--END-----\n");
}


int64_t alloc_empty_inode(
    int32_t id,				//i结点所属的用户
    int32_t type,			//文件类型，0-文件，1-目录
    int32_t sfd_id,			//i结点对应的目录id
    int32_t filelen,		//文件长度
    int32_t auth,		    //8个user的访问权限
    int32_t qcount          //文件的引用数
){
    int64_t allocated_inode_num = getFSInfo()->inode_num;

    if(allocated_inode_num>=(long long)(BLOCK_SIZE/sizeof(INode))){
        fprintf(stderr,"alloc_empty_inode: Can't allocate memory");
        return -1;
    }

    INode * inode = getIList() + allocated_inode_num;
    *inode = {
        id,
        type,
        sfd_id,
        filelen,
        auth,
        0,
        -1,
        qcount,
        time(NULL)
    };

    ++(getFSInfo()->inode_num);

    return allocated_inode_num;
}

int32_t make_directory(){//return inumber
    int64_t i_number = alloc_empty_inode(
       0,
       1,
       1,
       0,
       0777,
       1
    );
    if(i_number==-1){
        fprintf(stderr,"make_directory: Can't allocate empty inode.");
        return -1;
    }

    int32_t blockid = allocate_empty_block();
    if(blockid==-1){
        fprintf(stderr,"make_directory: Can't allocate empty block.");
        return -1;
    }

    INode * inode = getIList() + i_number;
    inode->filelen = _4KB;
    inode->diskBlockId = blockid;
    inode->diskBlockNum = 1;

    Directory * directory = getDirectory(inode->diskBlockId);
    DirectoryEntry *entry = directory->Entry;
    for(int i=0;i<BLOCK_SIZE/(int)sizeof(DirectoryEntry);++i){
        entry[i].inumber=-1;
    }

    return i_number;
}

char *format(long sz, long blocksz){//在内存中格式化文件系统
    char *tmp;
    if((tmp = (char *) malloc(sizeof(char)*sz))==NULL){
        fprintf(stderr,"format: Can't allocate memory");
        _exit(1);
    }
    setRawFs(tmp);

    int32_t empty_block_num = (sz/blocksz) - (ILIST_SIZE/blocksz) - 2;
    FSInfo *fs = getFSInfo();
    *fs = {
        sz,
        blocksz,
        0,
        empty_block_num,
        0
    };

    SuperBlock *sb = getSuperBlock();
    sb->empty_blocks_count = empty_block_num;
    for(int32_t i=0;i<empty_block_num;++i){
        sb->empty_blocks_no_stack[i] = i;
    }

    int32_t i_number = make_directory();

    if(i_number==-1){
        fprintf(stderr,"format: root_directory inode allocate failed.");
        _exit(1);
    }
    fs->inode_num = i_number;

    setRawFs(NULL);
    return tmp;
}

void presistent(const char *path, char *buf, unsigned long fileSystemSz){//将文件系统持久化到硬盘
    assert(path!=NULL);
    assert(buf!=NULL);

    FILE *file=NULL;
    if((file=fopen(path,"wb"))==NULL){
        fprintf(stderr,"presistent: File can't be open: %s",path);
        _exit(1);
    }

    if(fwrite(buf,sizeof(char),fileSystemSz,file)!=fileSystemSz){
        fprintf(stderr,"presistent: File allocating error: %s",path);
        _exit(2);
    }

    fflush(file);
    fclose(file);
}

char *transient(const char *path,unsigned long FSSize){
    assert(path!=NULL);

    char *tmp;
    if((tmp = (char *) malloc(sizeof(char)*FSSize))==NULL){
        fprintf(stderr,"transient: Can't allocate memeory");
        return NULL;
    }

    FILE *file =NULL;
    if((file=fopen(path,"rb"))==NULL){
        fprintf(stderr,"transient: File can't be open: %s",path);
        _exit(1);
    }

    if(fread(tmp,sizeof(char),FSSize,file)!=FSSize){
        fprintf(stderr,"transient: File read error: %s",path);
        _exit(2);
    }

    return tmp;
}

int32_t allocate_empty_block(){
    SuperBlock * sb = getSuperBlock();
    int64_t res = sb->empty_blocks_count;
    if(res>0){
        --(sb->empty_blocks_count);
        --(getFSInfo()->empty_block_num);
        return res-1;
    }else{
        return -1;
    }
}

INode *INumber2INode(int64_t inumber)
{
    if(inumber>=(long long)(BLOCK_SIZE/sizeof(INode))){
        return NULL;
    }else{
        return getIList() + inumber;
    }
}

int add_directory_entry(Directory *directory, const char *entryName, int64_t inumber)
{
    if(directory==NULL||entryName==NULL||inumber<0){
        fprintf(stderr,"add_directory_entry: error because directory or entryname or inumber is null.");
        return -1;
    }

    DirectoryEntry *entries = directory->Entry;
    for(int i=0;i<BLOCK_SIZE/(int)sizeof(DirectoryEntry);++i){
        if(entries[i].inumber==-1){
            strcpy_s(entries[i].name,56,entryName);
            entries[i].inumber = inumber;
            return 0;
        }
    }

    fprintf(stderr,"add_directory_entry: directory is full");
    return -1;

}

