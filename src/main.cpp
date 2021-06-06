#include <cstdlib>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cstring>
#include "myfs_fuse.h"

using std::string;
using std::vector;
static char pathOfFS[] = "/home/emon100/myfs.hd";

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


int main(int argc, char** argv){
    loadFileSystem(pathOfFS,_128MB);
    int ret;
    struct fuse_args args = FUSE_ARGS_INIT(argc,argv);

    struct fuse_operations fs_ops;
    memset(&fs_ops,0,sizeof(fuse_operations));
    fs_ops.readdir = &fs_readdir;
    fs_ops.getattr = &fs_getattr;
    fs_ops.read = &fs_read;
    fs_ops.write = &fs_write;
    fs_ops.rmdir = &fs_rmdir;
    fs_ops.mkdir = &fs_mkdir;
    fs_ops.create = &fs_create;
    fs_ops.unlink = &fs_unlink;
    fs_ops.flush = &fs_flush;

    ret = fuse_main(args.argc,args.argv,&fs_ops,NULL);
    fuse_opt_free_args(&args);
    return ret;
}
