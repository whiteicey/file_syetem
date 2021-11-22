/**
 * @file shell.h
 * @brief 定义交互终端的核心。
 * @version 0.1
 * @date 2021-11-14
 */

#ifndef __CMJFS__SHELL__SHELL_H__
#define __CMJFS__SHELL__SHELL_H__ 1

#include "../sys/user.h"

//! @addtogroup shell
//! @{

//! 工作目录的inode编号
extern int work_dir;
//! 当前登录的用户
extern user_t cnt_user;

//! @}

#endif // __CMJFS__SHELL__SHELL_H__
