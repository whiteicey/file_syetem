/**
 * @file stat.h
 * @brief 定义struct stat结构。
 * @version 0.1
 * @date 2021-11-14
 */

#ifndef __CMJFS__SYS_TYPES__STAT_H__
#define __CMJFS__SYS_TYPES__STAT_H__ 1

#include <time.h>

/**
 * @brief 描述一个inode的基本信息。
 * @ingroup fs_struct
 */
typedef struct stat {
    //! inode结点号
    int st_ino;
    //! 文件类型
    int st_mode;
    //! 引用计数
    int st_nlink;
    //! 文件大小
    int st_size;
    //! 文件所有者
    int st_uid;
    //! 文件属组
    int st_gid;
    //! 文件块大小
    int st_blksize;
    //! 文件块数量
    int st_blocks;
    //! 最后访问时间
    time_t st_atime;
    //! 最后修改时间
    time_t st_mtime;
    //! 状态改变时间
    time_t st_ctime;
} stat_t;

#endif // __CMJFS__SYS_TYPES__STAT_H__
