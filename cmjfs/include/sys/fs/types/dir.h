/**
 * @file dir.h
 * @brief 定义struct dir结构。
 * @version 0.1
 * @date 2021-11-14
 */

#ifndef __CMJFS__SYS_FS_TYPES__DIR_H__
#define __CMJFS__SYS_FS_TYPES__DIR_H__ 1

#include "dirent.h"

/**
 * @brief 描述目录结构。
 * 
 * 本质上是产生一个树形数据结构。<br>
 * 注意：记得依次释放dir_list的内容，以免内存泄漏。<br>
 * 
 * NOTE：ext4底层真的是这样的吗？我不确定，以后看看
 * 
 * @ingroup fs_struct
 */
typedef struct dir {
    //! 目录名称
    char dir_name[28];
    //! 目录大小
    int dir_list_size;
    //! 具体的目录列表
    dirent_t dir_list[20];
} dir_t;

#endif // __CMJFS__SYS_FS_TYPES__DIR_H__
