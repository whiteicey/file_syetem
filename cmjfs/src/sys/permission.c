/**
 * @file permission.c
 * @author xsf (bszxsf@gitee.com)
 * @version 0.1
 * @date 2021-11-14
 */

#include <sys/permission.h>
#include <sys/fs/mediactrl.h>

int access(const user_t* user, int ino, int mode) {
    // 若当前用户为root，直接返回0
    if (user->pw_uid == 0) {
        return 0;
    } if (user->pw_uid == inode[ino].i_stat.st_uid) { // 是文件所有者
        mode *= 0x100;
    } else if(user->pw_uid == inode[ino].i_stat.st_gid) { // 是组成员
        mode *= 0x10;
    }
    if((inode[ino].i_stat.st_mode & mode) != 0) {
        return 0;
    }
    return -1;
}
