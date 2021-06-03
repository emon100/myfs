#include <cstdio>
#include "syscall.h"

void ls(Directory *dir){
    DirectoryEntry * entries = dir->Entry;
    for(int i=0;i<BLOCK_SIZE/(int)sizeof(DirectoryEntry);++i){
        if(entries[i].inumber!=-1){
            printf("%s\t",entries[i].name);
        }
    }
}
