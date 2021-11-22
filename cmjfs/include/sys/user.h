/**
 * @file user.h
 * @brief 定义用户信息和用户操作。
 * @version 0.1
 * @date 2021-11-14
 */

#ifndef __CMJFS__SYS__USER_H__
#define __CMJFS__SYS__USER_H__ 1

/**
 * @brief 描述一个用户的信息。
 * @ingroup user
 */
typedef struct user {
    //! 用户名
    char pw_name[10];
    //! 明文密码
    char pw_passwd[20];
    //! 用户id
    int pw_uid;
    //! 用户所属组id
    int pw_gid;
} user_t;

/**
 * @brief 可用的下一个用户id。
 * 
 * 这本来是应该放到交互逻辑的，但这个设计下实在难以分离出去。
 * 
 * @ingroup user
 */
extern int idle_uid;

//! @addtogroup user_action
//! @{

/**
 * @brief 登录函数。
 * 
 * NOTE：在这里写文件交互？建议：外部写一个管理器，只加载一次。
 * 修改时管理器和文件同步修改。对大文件，这样效率比较高。
 * 
 * @param user 接收用户的登录参数
 * @param args 用户名。若为NULL，则执行交互式登录
 * @return int 成功返回0，失败返回-1。
 */
int login(user_t* user, const char* args);

/**
 * @brief 查找用户。
 * 
 * @param args 用户名
 * @return int 成功返回0，失败返回-1。
 */
int find_user(const char* args);

/**
 * @brief 添加用户。
 * 
 * @param user 执行操作的用户身份
 * @param args 待添加的用户名
 */
void add_user(const user_t* user, const char* args);
//  * NOTE：你这咋连返回值都没了？

//! @}

#endif // __CMJFS__SYS__USER_H__
