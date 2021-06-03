#include "gtest/gtest.h"
#include "my.h"

TEST(FileSystem, Transient){
    const int32_t FS_SIZE = _128MB;
    char *buf = transient("D:\\myfs.hd", FS_SIZE);
    SuperBlock *sb = (SuperBlock *)(buf + SUPERBLOCK_OFFSET);
    //EXPECT_EQ("SuperBlock empty_block_count",my_count,sb->emp);
    int blocksz = _4KB;
    int32_t empty_block_num = (FS_SIZE/blocksz) - (ILIST_SIZE/blocksz) - 2;
    EXPECT_TRUE(sb->empty_blocks_count==empty_block_num);
    EXPECT_TRUE(sb->empty_blocks_no_stack[10]==10);
}
