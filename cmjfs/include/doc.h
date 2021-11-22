/**
 * @file doc.h
 * @brief 帮助doxygen生成文档。**不要**试图包含该文件！
 * @version 0.1
 * @date 2021-11-14
 */

//! @defgroup config 配置项
//! @defgroup sys 核心系统
//! @{
//!   @defgroup fs 文件系统
//!   @{
//!     @defgroup fs_struct 结构
//!     @defgroup action FS行为
//!     @{
//!       @defgroup bitmap 位图
//!     @}
//!   @}
//!   @defgroup user 用户和权限
//!   @{
//!     @defgroup permission 权限
//!     @defgroup user_action 行为
//!   @}
//!   @defgroup shell 交互终端
//!   @{
//!     @defgroup io 交互接口
//!     @defgroup cmd 指令
//!     @{
//!       @defgroup cmd_action 指令行为
//!     @}
//!   @}
//! @}

// //! @defgroup wtf 完全不知道干什么用的东西

// /**
//  * @brief 兄啊，你连实现都没实现，我怎么会知道这是做什么的
//  * 
//  * @ingroup wtf
//  * 
//  * @param ino 希腊奶
//  * @param len 希腊奶
//  * @return char* 就不写con……哦这个不需要const啊，那没事了
//  */
// char* creat_reg(int ino, int len);

#error DO NOT include this file!

/**
 * 整体设计缺陷：
 * - 没有将用户的业务逻辑分离出交互逻辑，完全是按单用户系统设计，难以修改为多用户设计。换言之，耦合度过高。
 * - 参数命名令人困惑，明明不是指令却用arg。
 * - 文件系统逻辑和用户逻辑没有做切分。现实中底层主控对硬件肯定有绝对控制权，当然应当单独抽象。
 *   由于时间问题，这个工作量太大，没有修改。
 * - 容错不完善。
 * - 函数设计问题：大量未使用的参数，而它们有相当一部分是不必要的。
 * - 一切输出都通过stdout，这是坏习惯。
 * - 过多不明所以的硬编码设计，例如：
 * --- 10 （你自己应该明白是什么意思吧？）
 * --- dir_list的尺寸，你定义了宏但不用
 * --- dir_list保证0和1分别是当前目录和父目录，这本身没什么，但没有额外文档的话这种东西只能靠猜，不然根本看不懂
 *     （我的设计就没有且不可能有类似保证，设计与设计之间的差异很大，一定要讲清具体保证）
 *     这样的话，过两年连你自己也会看不懂自己写的啥
 * 代码问题：
 * - 别在行末加空格！
 * - 该加空格的地方不加
 * - 注释跟代码主体层次不够分明
 */
