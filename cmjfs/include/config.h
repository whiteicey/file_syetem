/**
 * @file config.h
 * @brief 定义基本数据，一部分可修改。
 * @version 0.1
 * @date 2021-11-14
 */

#ifndef __CMJFS__CONFIG_H__
#define __CMJFS__CONFIG_H__ 1

#include <limits.h>

//! @addtogroup config
//! @{

//! @brief 指定块尺寸。必须是2的幂。
#define BLOCK_SIZE 512
//! @brief 指定块数目。必须是正数。
#define BLOCK_NUM 512
//! 指定inode数目。必须是正数。
#define INODE_NUM 512
//! 指定直接索引数量。
#define INDIRECT_IDX_1_NUM 10
//! 用以限制一个文件夹下最多文件数
#define MAX_DIRLIST 30

//! @}

// 下面的是全局编译期检查。**不要修改**！！
// NOTE：用struct是为了避免产生实际符号，干扰编译过程。

// 检查是否非0，不是则返回负值。用于进行编译期检查
// NOTE：有些编译器可能不允许数组长度为0，但有些可以。我们利用这点实现编译期约束
// 原理：
// - 先把cond转化为数字0F/1T
// - 利用数组长度必须为正的特性，减去1后乘2，取相反数再加1
// - 则分别可以取到1和-1，目的达到
#define __CHECK_POS(cond) (1 - (((!!(cond)) - 1) << 1))
// 检查是否为0
#define __CHECK_NEG(cond) (1 - (((!(cond)) - 1) << 1))
// 检查x是否是2的幂，是则返回0，否则返回非0
#define __IS_POW_2(x) ((x) & ((x) - 1))

// 检查BLOCK_SIZE是否是2的幂。不满足的话就会在这里报错
struct __block_size_validator {
    // 约束1：BLOCK_SIZE必须是不小于8的正整数
    int _[__CHECK_POS(BLOCK_SIZE >= 8)];
    // 约束2：BLOCK_SIZE是2的幂
    // - 当x是2的幂时，x&(x-1)一定是0，反之非0
    // - 逻辑取反后，满足为1，反之为0
    int __[__CHECK_NEG(__IS_POW_2(BLOCK_SIZE))];
};

// 检查BLOCK_NUM是否是正数
struct __block_num_validator {
    int _[__CHECK_POS(BLOCK_NUM)];
};

// 检查INODE_NUM是否是正数
struct __inode_num_validator {
    int _[__CHECK_POS(INODE_NUM)];
};

// 验证INDIRECT_IDX_1_NUM
struct __indirect_idx_1_num_validator {
    int _[__CHECK_NEG(INDIRECT_IDX_1_NUM - 10)];
};

// 下面是依赖配置信息自动运算的部分，请勿修改

// blk_flag需要申请多少字节
#define BLK_FLAG_SIZE (((BLOCK_SIZE) + (CHAR_BIT - 1)) / CHAR_BIT)

#endif // __CMJFS__CONFIG_HH__
