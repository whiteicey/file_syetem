/**
 * @file command.h
 * @brief 提供交互指令。
 * @version 0.1
 * @date 2021-11-14
 */

#ifndef __CMJFS__SHELL__COMMAND_H__
#define __CMJFS__SHELL__COMMAND_H__ 1

//! @addtogroup cmd
//! @{

/**
 * @brief 描述一个指令条目。
 */
typedef struct cmd {
    //! 指令名称
    const char* name;
    //! 指令描述
    char* description;
    //! 指令回调函数
    void (*handler)(char* args);
} cmd_t;

/**
 * @brief 指令数。
 * 
 * 为了能分离实现和定义而写了硬编码。
 */
#define CMD_N 14
// TODO: 这样的话sizeof是不行的，需要另想办法。

/**
 * @brief 指令表。
 */
extern cmd_t cmd_table[];

//! @}

//! @addtogroup cmd_action
//! @{


void cmd_pwd(char*);
void cmd_ls(char*);
void cmd_mkdir(char*);
void cmd_cd(char*);
void cmd_rmdir(char*);
void cmd_su(char*);
void cmd_whoami(char*);
void cmd_useradd(char*);
void cmd_creat(char*);
void cmd_rm(char*);
void cmd_read(char*);
void cmd_write(char*);
void cmd_ln(char*);
void cmd_find(char*);

//! @}

#endif // __CMJFS__SHELL__COMMAND_H__
