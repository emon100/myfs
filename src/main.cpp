#include "my.h"

static char path[] = "D:\\myfs.hd";


int main(int argc, char** argv){
    const int32_t FS_SIZE = _10MB;
   // setRawFs(formatAndActivate(FS_SIZE,BLOCK_SIZE));
    //presistent(path, getRawFs(),FS_SIZE);
    setRawFs(transient(path, FS_SIZE));

    FSInfo *fs = getFSInfo();
    printFSInfo(fs);
    INode *rootINode = INumber2INode(fs->root_inumber);
    printINodeInfo(rootINode);

    /*
    //放一个空文件a到根目录
    INUMBER inumber = alloc_empty_inode(
        0,
        0,
        0,
        0,
        0777,
        1
    );

    Directory *rootDir = getDirectory(rootINode->diskBlockId);

    int err = add_directory_entry(rootDir,name, inumber);
    */
    const char * name = "a";

    //显示根文件夹
    INode *nd = INumber2INode(0);
    Directory * dir = getDirectory(nd->diskBlockId);
    printDirectory(dir);

    //找到这个文件的inumber
    INUMBER file = find_in_directory(dir,name);
    //显示这个文件的INode
    printINodeInfo(INumber2INode(file));
    //presistent(path, getRawFs(),FS_SIZE);
}
