/**
 * @file mediactrl.h
 * @brief 实现基本的介质管理。
 * @version 0.1
 * @date 2021-11-14
 */

#ifndef __CMJFS__SYS_FS__MEDIACTRL_H__
#define __CMJFS__SYS_FS__MEDIACTRL_H__ 1

#include "types/block.h"
#include "types/inode.h"

//! @addtogroup bitmap
//! @{
//! 设置标识位
#define SET_BLK_FLAG(i) (blk_flag[i / CHAR_BIT] |= '\x01' << (i % CHAR_BIT))
//! 检查标识位
#define TEST_BLK_FLAG(i) (blk_flag[i / CHAR_BIT] & '\x01' << (i % CHAR_BIT) ? 1 : 0)
//! 清除标识位
#define CLEAR_BLK_FLAG(i) (blk_flag[i / CHAR_BIT] ^= '\x01' << (i % CHAR_BIT))
//! @}

/**
 * @brief 全局块数据。
 * @ingroup fs
 */
extern block_t BLK[];

/**
 * @brief 位图管理，指示哪些块可用。
 * @ingroup bitmap
 */
extern char blk_flag[];

/**
 * @brief 全局inode数据。
 * @ingroup fs
 */
extern inode_t inode[];

/**
 * @brief 申请一个新块。
 * @ingroup action
 * 
 * NOTE：这个东西实际是申请而不是创建块，创建块是格式化做的事情。所以我把函数名改了。
 * 
 * @return int 如果成功，返回块号，否则返回-1。
 */
int acq_blk();

#endif // __CMJFS__SYS_FS__MEDIACTRL_H__
