/**
 * @file mediactrl.c
 * @version 0.1
 * @date 2021-11-14
 */

#include <string.h>
#include <sys/fs/mediactrl.h>

block_t BLK[BLOCK_NUM];
char blk_flag[BLK_FLAG_SIZE];
inode_t inode[INODE_NUM];

int acq_blk() {
    for (int i = 0; i < BLOCK_NUM; i++) {
        if (TEST_BLK_FLAG(i) == 0) {
            // 分配块
            SET_BLK_FLAG(i);
            // 块初始化
            // NOTE：我认为没有这个必要。真实的磁盘中，就我所知不会进行这样的初始化操作。
            // 这也是为什么能够进行数据恢复。
            memset(&BLK[i], 0, sizeof(block_t));
            return i;
        }
    }
    return -1;
}
