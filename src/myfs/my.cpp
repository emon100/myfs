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
    printf("fs_size :%lld\n",fs->fs_size);
    printf("block_size :%lld\n",fs->block_size);
    printf("inode_count :%lld\n",fs->inode_count);
    printf("empty_block_num :%lld\n",fs->empty_block_count);
    printf("root_inumber :%lld\n",fs->root_inumber);
    printf("----FSINFO--END-----\n");
}

void printINodeInfo(INode *i){
    printf("----INodeINFO-----\n");
    printf("id :%d\n",i->id);
    printf("type :%s\n",i->type?"directory":"file");
    printf("sfd :%d\n",i->sfd_id);
    printf("filelen :%d\n",i->filelen);
    printf("diskBlockNum :%d\n",i->diskBlockCount);
    printf("diskBlockId :%d\n",i->diskBlockId);
    printf("qcount :%d\n",i->qcount);
    printf("time :%lld\n",i->createTime);
    printf("----INodeINFO--END-----\n");
}

void printDirectory(Directory *dir){
    DirectoryEntry * entries = dir->Entry;

    printf("----Directory INFO-----\n");
    printf("name\t\tINumber\n");
    int count=0;
    for(int i=0;i<BLOCK_SIZE/(int)sizeof(DirectoryEntry);++i){
        if(entries[i].inumber!=-1){
            printf("%s\t\t%lld\n",entries[i].name,entries[i].inumber);
            ++count;
        }
    }
    printf("Total files: %d\n",count);
    printf("----INodeINFO--END-----\n");
}


INUMBER alloc_empty_inode(
    int32_t id,				//i结点所属的用户
    int32_t type,			//文件类型，0-文件，1-目录
    int32_t sfd_id,			//i结点对应的目录id
    int32_t filelen,		//文件长度
    int32_t auth,		    //9位，代表创建者，组的，其他用户的访问权限(rwx)
    int32_t qcount          //文件的引用数
){
    FSInfo *fsinfo = getFSInfo();
    int64_t allocated_inode_num = fsinfo->inode_count;

    if(allocated_inode_num>=(long long)(BLOCK_SIZE/sizeof(INode))){//检测是否还能分配
        fprintf(stderr,"alloc_empty_inode: Can't allocate memory");
        return -1;
    }

    //寻找空闲inode
    INode * inode = getIList() + allocated_inode_num;
    if(inode->qcount!=-1){
        inode = NULL;
        for(int i=allocated_inode_num+1;i<(long long)(BLOCK_SIZE/sizeof(INode));++i){
            if(getIList()[i].qcount==-1){
                inode = getIList() + i;
           }
        }
        if(inode==NULL){
            for(int i=0;i<allocated_inode_num;++i){
                if(getIList()[i].qcount==-1){
                    inode = getIList() + i;
                }
            }
        }
    }

    //
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

    ++(fsinfo->inode_count);
    return allocated_inode_num;
}

INUMBER dealloc_inode(INUMBER inumber){
    INode *inode = INumber2INode(inumber);
    if(inode != NULL){
        inode->qcount=-1;
        return inumber;
    }
    return -1;
}

INUMBER make_directory(INUMBER upperINumber){//return inumber
    if(upperINumber<0){
        return -1;
    }

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

    int32_t blockid = alloc_empty_block();
    if(blockid==-1){
        dealloc_inode(i_number);
        fprintf(stderr,"make_directory: Can't allocate empty block.");
        return -1;
    }

    INode * inode = INumber2INode(i_number);
    inode->filelen = _4KB;
    inode->diskBlockId = blockid;
    inode->diskBlockCount = 1;

    Directory * directory = getDirectory(inode->diskBlockId);
    DirectoryEntry *entry = directory->Entry;
    for(int i=0;i<BLOCK_SIZE/(int)sizeof(DirectoryEntry);++i){
        entry[i].inumber=-1;
    }
    entry[0].inumber=i_number;
    strncpy(entry[0].name,".",2);
    entry[1].inumber=upperINumber;
    strncpy(entry[1].name,"..",3);

    return i_number;
}

char *formatAndActivate(long sz, long blocksz){//在内存中格式化文件系统
    char *tmp;
    if((tmp = (char *) malloc(sizeof(char)*sz))==NULL){
        fprintf(stderr,"format: Can't allocate memory");
        exit(1);
    }
    setRawFs(tmp);

    int32_t empty_block_num = (sz-ILIST_SIZE)/BLOCK_SIZE - 2;
    FSInfo *fs = getFSInfo();
    *fs = {
        sz,
        blocksz,
        0,
        0,
        0
    };

    SuperBlock *sb = getSuperBlock();
    sb->empty_blocks_count = 0;
    for(int32_t i=0;i<empty_block_num;++i){
        deallocate_block(i);
        if(i%10000==0){
            printf("format: %d/%d\n",i,empty_block_num);
        }
    }

    assert(fs->empty_block_count==empty_block_num);

    for(int i=0;i<(long long)(BLOCK_SIZE/sizeof(INode));++i){
        getIList()[i].qcount=-1;
    }

    int32_t i_number = make_directory(0);
    if(i_number==-1){
        fprintf(stderr,"format: root_directory inode allocate failed.");
        exit(1);
    }
    fs->root_inumber = i_number;

    return tmp;
}

void presistent(const char *path, char *buf, unsigned long fileSystemSz){//将文件系统持久化到硬盘
    assert(path!=NULL);
    assert(buf!=NULL);

    FILE *file=NULL;
    if((file=fopen(path,"wb"))==NULL){
        fprintf(stderr,"presistent: File can't be open: %s",path);
        exit(1);
    }

    if(fwrite(buf,sizeof(char),fileSystemSz,file)!=fileSystemSz){
        fprintf(stderr,"presistent: File allocating error: %s",path);
        exit(2);
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
        free(tmp);
        return NULL;
    }

    if(fread(tmp,sizeof(char),FSSize,file)!=FSSize){
        fprintf(stderr,"transient: File read error: %s",path);
        free(tmp);
        return NULL;
    }

    return tmp;
}

int32_t alloc_empty_block(){
    SuperBlock * sb = getSuperBlock();
    int32_t sz = sb->empty_blocks_count;
    if(sz>1){//直接分配
        int32_t res = sb->empty_blocks_no_stack[sz-1];
        --(sb->empty_blocks_count);
        --(getFSInfo()->empty_block_count);
        return res;
    }else{//需要分配组长块
        FSInfo *fsi = getFSInfo();
        if(fsi->empty_block_count>=1){//还有许多块
            int32_t res = sb->empty_blocks_no_stack[0];
            memcpy(sb,getBlock(res),sizeof(SuperBlock));
            --(fsi->empty_block_count);
            return res;
        }else{//没有任何空闲块了
            return -1;
        }
    }
}

int32_t deallocate_block(int32_t blockid){
    if(blockid<0){
        return -1;
    }
    SuperBlock *sb=getSuperBlock();
    int32_t sz = sb->empty_blocks_count;
    const int32_t maxcount = BLOCK_SIZE/sizeof(int32_t) - 1;
    if(sz<maxcount){
        sb->empty_blocks_no_stack[sz]=blockid;
        ++(sb->empty_blocks_count);
        ++(getFSInfo()->empty_block_count);
    }else{
        memcpy(getBlock(blockid),sb,sizeof(SuperBlock));
        sb->empty_blocks_no_stack[0]=blockid;
        sb->empty_blocks_count=1;
        ++(getFSInfo()->empty_block_count);
    }
    return 0;
}

INode *INumber2INode(int64_t inumber)
{
    if(inumber<0 || inumber>=(long long)(ILIST_SIZE/sizeof(INode))){
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
    int good_entry = -1;

    for(int i=0;i<BLOCK_SIZE/(int)sizeof(DirectoryEntry);++i){
        if(entries[i].inumber==-1){
            good_entry = i;
        }else{
            if(strcmp(entryName,entries[i].name)==0){
                fprintf(stderr,"add_directory_entry: file already exists.");
                return -2;
            }
        }
    }

    if(good_entry!=-1){
        strncpy(entries[good_entry].name,entryName,56);
        entries[good_entry].inumber = inumber;
        return 0;
    }

    fprintf(stderr,"add_directory_entry: directory is full");
    return -3;
}

INUMBER find_in_directory(Directory *dir, const char *name){
    if(dir==NULL || name==NULL){
        return -1;
    }
    for(int i=0;i<BLOCK_SIZE/(int)sizeof(DirectoryEntry);++i){
        if(dir->Entry[i].inumber!=-1&&strcmp(dir->Entry[i].name,name)==0){
            return dir->Entry[i].inumber;
        }
    }
    return -1;
}

/*
int give_file_an_empty_block(INode *inode){
    if(inode==NULL){
        return -1;
    }

    int block_id = alloc_empty_block();
    if(block_id==-1){
        fprintf(stderr,"alloc_empty_block 1 failed.");
        return -1;
    }

    if(inode->diskBlockCount==1){
        int one_more_block = alloc_empty_block();
        if(one_more_block==-1){
            fprintf(stderr,"alloc_empty_block 2 failed.");
            deallocate_block(block_id);
            return -1;
        }
        SuperBlock *b = (SuperBlock *)getBlock(block_id);

    }else{


    }
}
*/
