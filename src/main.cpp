#include <cstdlib>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cstring>
#include "my.h"
#include "syscall.h"
#define USE_FUSE_VERSION 31
#include "fuse3/fuse.h"

using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::cin;
static char pathOfFS[] = "/home/emon100/myfs.hd";

INUMBER currentDir;
vector<string> currentDirString;


void ls_final()
{
    ls(getDirectory(INumber2INode(currentDir)->diskBlockId));
}

void mkdir_final()
{
    Directory *dir=getDirectory(INumber2INode(currentDir)->diskBlockId);
    cout<<"please input mkdir filename"<<endl;
    string newfilename;
    cin>>newfilename;
    mkdir(currentDir,newfilename.c_str());
    cout<<"mkdir success"<<endl;
    ls_final();
}

void chdir_final()
{
    //Directory *dir=getDirectory(INumber2INode(currentDir)->diskBlockId);
    cout<<"please input chdir path"<<endl;
    string newpath;
    cin>>newpath;
    chdir(currentDir,newpath);
    cout<<"chdir success"<<endl;
    ls_final();
}


static int fs_readdir(const char *path,void *buf, fuse_fill_dir_t filler,
        off_t off,struct fuse_file_info *fi,
        enum fuse_readdir_flags flags){

    int r = chdir(0,path);
    if(r<0) return r;

    DirectoryEntry *entries = getDirectory(INumber2INode(r)->diskBlockId)->Entry;

    for(int i=0;i<BLOCK_SIZE/(int)sizeof(DirectoryEntry);++i){
        if(entries[i].inumber!=-1){
            filler(buf,entries[i].name,NULL,0,FUSE_FILL_DIR_PLUS);
        }
    }
    return 0;
}

static int fs_getattr(const char *path, struct stat *buf, struct fuse_file_info *fi){
    memset(buf,0,sizeof(struct stat));
    buf->st_mode = S_IFDIR|0777;

    int r = inumber_of_path(getFSInfo()->root_inumber,path);
    if(r<0) return r;

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

vector<string> getdirString(INUMBER now){
    vector<string> result;

    Directory *d =getDirectory(INumber2INode(now)->diskBlockId);
    INUMBER upperINUMBER= find_in_directory(d,"..");//上层inumber

    while(upperINUMBER!=now){
        Directory *upperDirectory = getDirectory(INumber2INode(upperINUMBER)->diskBlockId);//上层目录
        DirectoryEntry *entry =  find_entry_in_directory_by_INUMBER(upperDirectory,now);
        result.push_back(string(entry->name));

        now = upperINUMBER;//now变为原来的上层
        upperINUMBER = find_in_directory(upperDirectory,"..");//上层变为上上层
    }

    result.push_back("/");
    reverse(result.begin(),result.end());
    return result;
}


int loadFileSystem(char *path,int32_t FS_SIZE){
    char *rawfs =  transient(path, FS_SIZE);
    if(rawfs == NULL){
        return -1;
    }
    setRawFs(rawfs);
    return 0;
}

void demo1(){
    const int32_t FS_SIZE = _128MB;
    setRawFs(formatAndActivate(FS_SIZE,BLOCK_SIZE));
    presistent(pathOfFS, getRawFs(),FS_SIZE);
}

void demo2(){
    const int32_t FS_SIZE = _128MB;
    loadFileSystem(pathOfFS,FS_SIZE);

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


    presistent(pathOfFS, getRawFs(),FS_SIZE);//TODO
    printf("try chdir\n");
    INUMBER b = chdir(0,"/b");
    printINodeInfo(INumber2INode(b));
    INUMBER c = chdir(0,"/b/c");
    printINodeInfo(INumber2INode(c));
    vector<string> v = getdirString(c);
    for(auto &&s:v){
        printf("%s\n",s.c_str());
    }
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
    loadFileSystem(pathOfFS,FS_SIZE);

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

int fs_read(const char *path, char *buf, size_t size, off_t off, struct fuse_file_info *fi){
    inumber_of_path(0,path);
    int r = inumber_of_path(getFSInfo()->root_inumber,path);
    if(r<0) return r;

    INode *i = INumber2INode(r);
    if(i&&i->type!=1){
        return read(r,off,buf,size);
    }else{
        return -ENOENT;
    }
}

int fs_write(const char *path,const char *buf, size_t size, off_t off, struct fuse_file_info *fi){
    inumber_of_path(0,path);
    int r = inumber_of_path(getFSInfo()->root_inumber,path);
    if(r<0) return r;

    INode *i = INumber2INode(r);
    if(i&&i->type!=1){
        r = write(r,off,(void *)buf,size);
        if(r>0) presistent(pathOfFS,getRawFs(),getFSInfo()->fs_size);
        return r;
    }else{
        return -ENOENT;
    }
}


int main(int argc, char** argv){
    // demo1();
    // demo2();
    //demo3();
    loadFileSystem(pathOfFS,_128MB);
    int ret;
    struct fuse_args args = FUSE_ARGS_INIT(argc,argv);

    struct fuse_operations fs_ops;
    memset(&fs_ops,0,sizeof(fuse_operations));
    fs_ops.readdir = &fs_readdir;
    fs_ops.getattr = &fs_getattr;
    fs_ops.read = &fs_read;
    fs_ops.write = &fs_write;


    ret = fuse_main(args.argc,args.argv,&fs_ops,NULL);
    fuse_opt_free_args(&args);
    return ret;
}
