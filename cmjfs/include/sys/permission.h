/**
 * @file permission.h
 * @brief 文件权限的验证操作。
 * @version 0.1
 * @date 2021-11-14
 */

#ifndef __CMJFS__SYS_FS__PERMISSION_H__
#define __CMJFS__SYS_FS__PERMISSION_H__ 1

#include "user.h"

//! @addtogroup permission
//! @{

//! 属主读权限掩码
#define S_IRUSR 0x00400
//! 属主写权限掩码
#define S_IWUSR 0x00200
//! 属主执行权限掩码
#define S_IXUSR 0x00100
//! 属组读权限掩码
#define S_IRGRP 0x00040
//! 属组写权限掩码
#define S_IWGRP 0x00020
//! 属组执行权限掩码
#define S_IXGRP 0x00010
//! 其他用户读权限掩码
#define S_IROTH 0x00004
//! 其他用户写权限掩码
#define S_IWOTH 0x00002
//! 其他用户执行权限掩码
#define S_IXOTH 0x00001
//! 读权限掩码
#define R_OK 0x04
//! 写权限掩码
#define W_OK 0x02
//! 执行权限掩码
#define X_OK 0x01

//! 检查文件权限
#define HAVE_ACCESS(ino,mode,ch) (inode[ino].i_stat.st_mode & mode ? ch : '-')

// TODO: 三种特殊权限码

/**
 * @brief 验证用户权限。
 * 
 * @param user 用户对象的指针
 * @param ino 文件的inode编号
 * @param mode 访问模式
 * @return int 成功返回0，失败或无权限返回-1。
 */
int access(const user_t* user, int ino, int mode);

//! @}

#endif // __CMJFS__SYS_FS__PERMISSION_H__
