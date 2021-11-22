/**
 * @file fsops.h
 * @brief 文件系统的核心操作。
 * @version 0.1
 * @date 2021-11-14
 */

#ifndef __CMJFS__SYS_FS__FS_H__
#define __CMJFS__SYS_FS__FS_H__ 1

#include "types/dir.h"
#include "../user.h"

//! @addtogroup action
//! @{

//! 普通文件标识
#define S_IFREG 0x0100000
//! 目录文件标识
#define S_IFDIR 0x0040000
//! 递归行为标识
#define RECURSIVE 0x800

//! 获取文件的第n个块
#define GET_BLKN(ino,n) (BLK[inode[ino].blk1[n]])
// NOTE：这，，，真就只有直接索引？
// TODO: 修改这个东西，我猜可能没法用宏来实现

//! 获取文件的第n个块
#define GET_PAGE(ino,page) (page >= 10 ? BLK[inode[ino].blk2].index[page-10] : inode[ino].blk1[page])
//! 获取字符(？)
#define GET_CHAR(ino,page,offset) (BLK[GET_PAGE(ino,page)].str[offset])
// NOTE：宏必须加括号，这是原则。这里我帮你加上了。这样的原则是为了防止疏忽和减少维护成本。
// 以及，为什么是page不是block？

/**
 * @brief 为文件申请新的数据块。
 * 
 * @param ino 文件的inode
 * @return int 新数据块的id，失败返回-1。
 */
int add_blk_for_file(int ino);

/**
 * @brief 在指定目录下查找具有指定名称的文件。
 * 
 * @param ino 目录项的inode编号
 * @param name 待查的文件名
 * @param mode 查询模式
 * @return int 若找到文件，返回inode编号，否则返回-1。
 */
int find_name_in_dir(int ino, const char* name, int mode);

/**
 * @brief 创建文件的stat。
 * 
 * @param user 文件属主
 * @param ino 文件获得的inode编号
 * @param mode 创建模式
 * @return int 总是返回0。
 */
int creat_stat(const user_t* user, int ino, int mode);

/**
 * @brief 为文件创建目录入口。
 * 
 * 具体而言：在fino指向的目录下，为ino文件分配一个名为name的入口。
 * 
 * @param fino 父目录的inode编号
 * @param ino 子目录的inode编号
 * @param name 文件名
 * @return int 成功返回0，否则返回-1。
 */
int creat_dirent(int fino, int ino, const char* name);

/**
 * @brief 为文件分配struct dir结构。
 * 
 * 会分配.和..目录。
 * 
 * NOTE：你给我的注释写着返回块号，但你自己看看，你返回了锤子块号
 * 
 * @param ino 当前目录的inode
 * @param f_ino 父目录的inode
 * @param dir_name 当前目录文件名
 * @return int 成功返回0，否则返回-1。
 */
int creat_dir(int ino, int f_ino, const char* dir_name);
// TODO: 检查潜在bug

/**
 * @brief 分配inode。
 * 
 * @return int 成功返回inode编号，失败返回-1。
 */
int creat_ino();

/**
 * @brief 创建文件。
 * 
 * @param user 文件属主
 * @param cwd 工作目录
 * @param args 参数（？不应该是name？）
 * @param mode 文件权限
 * @return int 已有文件但不可写返回0，创建失败返回-1，成功创建返回inode编号。
 */
int creat(const user_t* user, int cwd, const char* args, int mode);

// TODO: 你truncate呢？？这个调用很重要啊

/**
 * @brief 创建目录。
 * 
 * @param user 目录属主
 * @param cwd 工作目录
 * @param dir_name 目录名
 * @param mode 你尽管改，起作用算我输
 * @return int 成功返回0，否则返回-1。
 */
int mkdir(const user_t* user, int cwd, const char* dir_name, int mode);

/**
 * @brief 查看目录信息。
 * 
 * NOTE：真正的ls指令应该和权限有关。执行权限被拒绝的话应该不能成功执行ls。我不太确定（
 * 
 * @param ino 要查看的目录的inode编号。
 * @param args 希腊奶，传NULL就完事了！
 */
void ls(int ino, char* args);

/**
 * @brief 取得指定inode的目录名。
 * 
 * @param cwd 要取得目录名的inode编号
 * @param buf 缓冲区
 * @param ino 递归参数，一般填当前目录即可
 */
void pwd(int cwd, char* buf, int ino);

/**
 * @brief 修改指定用户的工作目录。
 * 
 * @param user 用户
 * @param cwd 接收修改后工作目录的inode编号
 * @param dir_name 目标目录
 */
void cd(const user_t* user, int* cwd, const char* dir_name);

/**
 * @brief 删除指定目录下的第n项。
 * 
 * NOTE：讲真，这个是不是不应该暴露出来？
 * 
 * @param d 待处理的目录
 * @param n 见介绍
 * @return int 总是返回0。
 */
int rm_dir_item(dir_t* d, int n);

/**
 * @brief 删除指定文件。
 * 
 * @param user 用户
 * @param cwd 工作目录
 * @param args 目标文件名？
 * @param workIno 希腊奶（？）
 * @param mode 删除模式
 */
void rm(const user_t* user, int cwd, const char* args, int workIno, int mode);

/**
 * @brief 删除目录。
 * 
 * @param user 用户
 * @param cwd 工作目录
 * @param dir_name 目标目录名
 * @param mode 删除模式
 */
void rmdir(const user_t* user, int cwd, char* dir_name, int mode);

/**
 * @brief 创建文件。
 * 
 * @param user 创建者
 * @param cwd 工作目录
 * @param args 文件名
 */
void creat_file(const user_t* user, int cwd, const char* args);

/**
 * @brief 打开文件。
 * 
 * @param user 执行操作的用户
 * @param cwd 工作目录
 * @param args 待查文件名
 * @return int 成功返回inode，否则返回-1。
 */
int open_file(const user_t* user, int cwd, const char* args);

/**
 * @brief 读取文件。
 * 
 * @param user 用户
 * @param cwd 工作目录
 * @param argc 没有用
 * @param argv 参数
 * @return int 成功返回0，否则返回-1。
 */
int read_file(const user_t* user, int cwd, int argc, const char* argv[]);

/**
 * @brief 写入文件。
 * 
 * @param user 用户
 * @param cwd 工作目录
 * @param argc 没有用
 * @param argv 参数
 * @return int 成功返回0，失败返回-1。
 */
int write_file(const user_t* user, int cwd, int argc, const char* argv[]);

/**
 * @brief 创建硬链接。
 * 
 * 接受两个参数，第一个参数是源文件名，第二个参数是硬链接的文件名。
 * 
 * @param cwd 工作目录
 * @param argc 没有用
 * @param argv 参数
 * @return int 成功返回0，失败返回-1。
 */
int link_file(int cwd, int argc, const char* argv[]);

/**
 * @brief 查找文件。
 * 
 * @param cwd 工作目录
 * @param args 参数
 * @param ino inode编号
 */
void find_file(int cwd, const char* args, int ino);

//! @}

#endif // __CMJFS__SYS_FS__FS_H__
