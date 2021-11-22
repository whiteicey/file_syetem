/**
 * @file dirents.h
 * @brief 定义struct dirents。
 * @version 0.1
 * @date 2021-11-14
 */

#ifndef __CMJFS__SYS_FS_TYPES_H__
#define __CMJFS__SYS_FS_TYPES_H__ 1

/**
 * @brief 扩展目录结构。
 * @ingroup fs_struct
 */
typedef struct dirents {
    //! 目录项数目
    int dir_list_size;
    //! 具体目录项列表
    dirent_t dirlist[21];
} dirents_t;

#endif // __CMJFS__SYS_FS_TYPES_H__
