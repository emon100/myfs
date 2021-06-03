#ifndef MY_H
#define MY_H
#include <cstdint>

typedef int64_t INUMBER;

const int32_t _128MB = 1<<27;
const int32_t _8MB = 1<<23;
const int32_t _2MB = 1<<21;
const int32_t _4KB = 1<<12;

const int32_t _10MB = _8MB + _2MB;
const int32_t BLOCK_SIZE = _4KB;
const int32_t ILIST_SIZE = _8MB;
//const int MAX_INODE_NUM =  ;
//const int MAX_BLOCK_NUM = ; //除超级块以外的块

const int32_t FSINFO_OFFSET = 0;
const int32_t SUPERBLOCK_OFFSET = 1 * BLOCK_SIZE;
const int32_t ILIST_OFFSET = SUPERBLOCK_OFFSET + 1 * BLOCK_SIZE;
const int32_t EMPTY_OFFSET = ILIST_OFFSET + ILIST_SIZE;

typedef struct FSInfo {
    int64_t fs_size;//文件系统大小
    int64_t block_size;//块大小
    int64_t inode_count;//当前文件数量
    int64_t empty_block_count;//空块数量
    INUMBER root_inumber;//根目录的inumber
} FSInfo;

typedef struct SuperBlock{
    int32_t empty_blocks_count;
    int32_t empty_blocks_no_stack[BLOCK_SIZE/sizeof(empty_blocks_count) - 1];
}SuperBlock;

typedef struct INode{
    int32_t id;				//i结点所属的用户
    int32_t type;			//文件类型，0-文件，1-目录
    int32_t sfd_id;			//i结点对应的目录id
    int32_t filelen;		//文件长度
    int32_t auth;		    //9位，代表创建者，组的，其他用户的访问权限(rwx)
    int32_t diskBlockNum;   //文件占用磁盘块个数
    int32_t diskBlockId;    //所占磁盘块的id号的索引块
    int32_t qcount;         //文件的引用数// -1代表空
    int64_t createTime;     //文件创建时间
} INode;

typedef struct DirectoryEntry{
    INUMBER inumber; //为-1代表无文件
    char name[56];
}DirectoryEntry;

typedef struct Directory{
    DirectoryEntry Entry[BLOCK_SIZE/sizeof(DirectoryEntry)]; //64个文件项
}Directory ;

char *getRawFs();//获得文件系统的首地址
void setRawFs(char *);//设置文件系统的首地址
inline FSInfo *getFSInfo(){
    return  (FSInfo *) (getRawFs()+FSINFO_OFFSET);
}

inline INode *getIList(){
    return (INode *)(getRawFs() + ILIST_OFFSET);
}

inline SuperBlock *getSuperBlock(){
    return (SuperBlock *)(getRawFs() + SUPERBLOCK_OFFSET);
}

inline Directory *getDirectory(int32_t blockID){
    return (Directory *) (getRawFs() + (EMPTY_OFFSET + blockID * BLOCK_SIZE));
}

void printFSInfo(FSInfo *fs);
void printINodeInfo(INode *fs);
void printDirectory(Directory *directory);

char *formatAndActivate(long sz, long blocksz);//在内存中格式化文件系统
void presistent(const char *path, char *buf, unsigned long fileSystemSz);//将文件系统持久化到外存
char *transient(const char *path, unsigned long FSSize);//从外存把文件系统载入内存


int32_t allocate_empty_block();//分配空磁盘块，返回磁盘块号，失败返回-1
INUMBER alloc_empty_inode( //分配并设置一个空inode，返回inumber，失败返回-1
    int32_t id,				//i结点所属的用户
    int32_t type,			//文件类型，0-文件，1-目录
    int32_t sfd_id,			//i结点对应的目录id
    int32_t filelen,		//文件长度
    int32_t auth,		    //9位，代表创建者，组的，其他用户的访问权限(rwx)
    int32_t qcount          //文件的引用数
);

INUMBER make_directory(); //给目录文件分配1个磁盘块和inode，返回inumber
INode *INumber2INode(INUMBER inumber); //把inumber转换成inode *
int add_directory_entry(Directory *directory,const char * entryName, INUMBER inumber);//添加目录项，失败返回-1

void ls(Directory *d);
INUMBER find_in_directory(Directory *dir, const char *path);


#endif // MY_H
