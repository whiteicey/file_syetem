/**
 * @file dirent.h
 * @brief 定义struct dirent结构。
 * @version 0.1
 * @date 2021-11-14
 */

#ifndef __CMJFS__SYS_FS_TYPES__DIRENT_H__
#define __CMJFS__SYS_FS_TYPES__DIRENT_H__ 1

/**
 * @brief 描述一个项目的入口。
 * @ingroup fs_struct
 */
typedef struct dirent{
    //! 关联的inode编号
    int d_ino;
    //! 项目的名称
    char d_name[20];
} dirent_t;

#endif // __CMJFS__SYS_FS_TYPES__DIRENT_H__
