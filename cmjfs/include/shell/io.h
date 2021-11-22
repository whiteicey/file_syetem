/**
 * @file io.h
 * @brief 提供交互功能。
 * @version 0.1
 * @date 2021-11-14
 */

#ifndef __CMJFS__SHELL__IO_H__
#define __CMJFS__SHELL__IO_H__ 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//! @addtogroup io
//! @{

/**
 * @brief 读入指令。
 * 
 * @param str 命令提示符（prompt）
 * @return char* 读到的指令。
 */
char* readline(const char* str);

/**
 * @brief 向指令历史添加指令。（未完成）
 * 
 * @param p 待添加的指令
 */
void add_history(char* p);

/**
 * @brief 接受输入字符。不可重入。
 * 
 * 尚未完成的读取历史指令
 * 
 * @return char* 读入的指令
 */
char* rl_gets();

//! @}

#endif // __CMJFS__SHELL__IO_H__
