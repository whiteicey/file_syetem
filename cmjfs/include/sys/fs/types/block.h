/**
 * @file block.h
 * @brief 定义文件系统块。
 * @version 0.1
 * @date 2021-11-14
 */

#ifndef __CMJFS__SYS_FS_TYPES__BLOCK_H__
#define __CMJFS__SYS_FS_TYPES__BLOCK_H__ 1

#include "../../../config.h"
#include "dir.h"
#include "dirents.h"

/**
 * @brief 定义文件系统数据块。
 * 
 * NOTE：我们真的**非常不建议**这样使用union！初始化一个域后访问其他域是UB。
 * 建议额外用一个字段记录union里面存的到底是什么，或者干脆改用类似void*的东西。
 * 
 * NOTE2：鉴于实现比较怪（真正的dentry肯定不是这么存的），不进行block_t是否能存下dir_t和dirents_t的校验。
 * 
 * @ingroup fs_struct
 */
typedef union block {
    //! 数据块
    char str[BLOCK_SIZE];
    //! 目录块
    dir_t dentry;
    //! 索引块
    int index[BLOCK_SIZE / sizeof(int)];
    //! 扩展目录结构
    dirents_t dentrys;
} block_t;

#endif // __CMJFS__SYS_FS_TYPES__BLOCK_H__
