#include "gtest/gtest.h"
#include "my.h"
#include "syscall.h"

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
/*

void demo1(){
    const int32_t FS_SIZE = _128MB;
    setRawFs(formatAndActivate(FS_SIZE,BLOCK_SIZE));
}

void demo2(){
    const int32_t FS_SIZE = _128MB;
    setRawFs(transient("/home/emon100/myfs.hd",FS_SIZE));

    FSInfo *fs = getFSInfo();
    printf("output filesystem info\n");
    printFSInfo(fs);
    INode *rootINode = INumber2INode(fs->root_inumber);
    printINodeInfo(rootINode);
    printf("\n\n\n");

    //放一个空文件a到根目录
    INUMBER inumber = alloc_empty_inode(
        0,
        0,
        0,
        0,
        0777,
        0
    );

    Directory *rootDir = getDirectory(rootINode->diskBlockId);

    const char * name = "a";
    int err = add_directory_entry(rootDir,name, inumber);

    printf("try write and read\n");
    char buf[50] = "Hello world";
    write(inumber,0,buf,50);

    err = add_directory_entry(rootDir,name, inumber);
    //找到这个文件的inumber

    INUMBER file = find_in_directory(rootDir,name);
    char result[50] = {};
    read(file,0,result,50);

    printf("\n%s\n",result);

    printf("\n\n\n");

    printf("try create two dirs in root\n");
    const char * dirname = "b";
    INUMBER B_Directory = make_directory(0);
    add_directory_entry(rootDir,dirname,B_Directory);
    INode *bnd = INumber2INode(B_Directory);

    INUMBER C_Directory = make_directory(B_Directory);
    add_directory_entry(getDirectory(bnd->diskBlockId),"c",C_Directory);

    //显示根文件夹
    printINodeInfo(rootINode);
    printf("\n\n\n");


    printf("try chdir\n");
    INUMBER b = chdir(0,"/b");
    printINodeInfo(INumber2INode(b));
    INUMBER c = chdir(0,"/b/c");
    printINodeInfo(INumber2INode(c));
    printf("\n\n\n");


    printf("try rmdir\n");
    printf("%d",rmdir(rootDir,"/a"));
    printINodeInfo(rootINode);
    printf("\n\n\n");
    printf("try rm a file\n");
    printf("%d",rm(rootDir,"/a"));
    printINodeInfo(rootINode);
    printf("\n\n\n");

    printf("try rmdir /b/c\n");
    printINodeInfo(bnd);
    printf("%d",rmdir(rootDir,"b/c"));
    printINodeInfo(bnd);
    printf("\n\n\n");
}

void demo3(){
    const int32_t FS_SIZE = _128MB;
    setRawFs(transient("/home/emon100/myfs.hd",FS_SIZE));

    FSInfo *fs = getFSInfo();
    printf("output filesystem info\n");
    printFSInfo(fs);
    INode *rootINode = INumber2INode(fs->root_inumber);
    printINodeInfo(rootINode);
    printf("\n\n\n");

    Directory *root = getDirectory(rootINode->diskBlockId);

    printf("try write and read\n");
    INUMBER file = find_in_directory(root,"a"); //找到a文件的inumber
    char result[50] = {};
    read(file,0,result,50);

    printf("\n%s\n",result);

    printf("\n\n\n");


    printf("try allocate empty blocks\n");
    //显示这个文件的INode
    INode *inodefile = INumber2INode(file);
    give_file_an_empty_block(inodefile);
    give_file_an_empty_block(inodefile);
    give_file_an_empty_block(inodefile);
    give_file_an_empty_block(inodefile);
    give_file_an_empty_block(inodefile);
    give_file_an_empty_block(inodefile);
    give_file_an_empty_block(inodefile);
    give_file_an_empty_block(inodefile);
    give_file_an_empty_block(inodefile);
    give_file_an_empty_block(inodefile);
    give_file_an_empty_block(inodefile);
    printINodeInfo(inodefile);
    printFSInfo(getFSInfo());
    printf("\n\n");

    printf("try allocate file\n");
    rm(root,"a");
    printFSInfo(getFSInfo());
    printf("\n\n");

}

*/
