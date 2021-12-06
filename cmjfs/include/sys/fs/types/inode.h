/**
 * @file inode.h
 * @brief 定义inode。
 * @version 0.1
 * @date 2021-11-14
 */

#ifndef __CMJFS__SYS_FS_TYPES__INODE_H__
#define __CMJFS__SYS_FS_TYPES__INODE_H__ 1

#include "stat.h"

/**
 * @brief 描述inode结点。
 * @ingroup fs_struct
 */
typedef struct inode {
    //! 结点状态信息
    stat_t i_stat;
    //! 直接索引
    int blk1[INDIRECT_IDX_1_NUM];
    //! 一级索引
    int blk2;
} inode_t;

#endif // __CMJFS__SYS_FS_TYPES__INODE_H__
