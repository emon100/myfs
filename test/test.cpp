#include "gtest/gtest.h"
#include "my.h"

TEST(FileSystem, Transient){

    const int32_t FS_SIZE = _10MB;
    setRawFs(transient("D:\\okmyfs.hd", FS_SIZE));
    SuperBlock *sb = getSuperBlock();

    EXPECT_EQ(sb->empty_blocks_count,getFSInfo()->empty_block_count)<<"SuperBlock empty_block_count should b";;
    EXPECT_EQ(sb->empty_blocks_no_stack[10],10);
}

TEST(FileSystem, createdir){
    const int32_t FS_SIZE = _10MB;
    setRawFs(formatAndActivate(FS_SIZE,BLOCK_SIZE));

    FSInfo *fs = getFSInfo();

    INode *rootINode = INumber2INode(fs->root_inumber);
    EXPECT_EQ(rootINode->id , 0);
    EXPECT_EQ(rootINode->type , 1);
    EXPECT_EQ(rootINode->sfd_id ,1);
    EXPECT_EQ(rootINode->filelen, _4KB);
    EXPECT_EQ(rootINode->auth,  0777);
    EXPECT_EQ(rootINode->qcount,  1);


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

    EXPECT_STRCASEEQ(rootDir->Entry[0].name,".");
    EXPECT_STRCASEEQ(rootDir->Entry[1].name,"..");
    EXPECT_EQ(rootDir->Entry[0].inumber,0);
    EXPECT_EQ(rootDir->Entry[1].inumber,0);

    const char * name = "a";
    int err = add_directory_entry(rootDir,name, inumber);


    //显示根文件夹
    INode *nd = INumber2INode(0);
    Directory * dir = getDirectory(nd->diskBlockId);
    EXPECT_STRCASEEQ(rootDir->Entry[2].name,"a");

    //找到这个文件的inumber
    INUMBER file = find_in_directory(dir,name);
}
