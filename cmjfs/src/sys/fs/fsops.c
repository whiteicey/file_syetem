/**
 * @file fsops.c
 * @version 0.1
 * @date 2021-11-14
 */

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fs/types/dir.h>
#include <sys/permission.h>
#include <sys/fs/mediactrl.h>
#include <sys/fs/fsops.h>

// 本来不该include这个，但没办法，write_file引用了io组件，耦合度太高了
#include <shell/io.h>

// 不知道是做什么的，但总之似乎不需要暴露接口
#define NOTDEFAULTDIR(name) (strcmp(name,".") != 0 && strcmp(name,"..") != 0)

int add_blk_for_file(int ino) {
    int num = acq_blk();
    if (num == -1) {
        return -1;
    }
    // 将st_blocks+1
    if (inode[ino].i_stat.st_blocks < 10) {
        inode[ino].blk1[inode[ino].i_stat.st_blocks++] = num;
    // NOTE：位运算的优先级非常低，但你忘了加括号。以后要注意。
    } else if (inode[ino].i_stat.st_blocks == 10 && (inode[ino].i_stat.st_mode & S_IFDIR) == 0) {
        // 文件直接索引已满，此时文件不能是目录
        inode[ino].blk2 = num;
        num = acq_blk();
        if ((BLK[inode[ino].blk2].index[inode[ino].i_stat.st_blocks++ - 10] = num) == -1) {
            return -1;
        }
    } else {
        // 此时需要将新分配的块加入到一级索引
        BLK[inode[ino].blk2].index[inode[ino].i_stat.st_blocks++ - 10] = num;
    }
    return num;
}

// 就不加const是吧
int find_name_in_dir(int ino, const char* name, int mode) {
    //判断ino所指向的文件是否为目录
    if ((inode[ino].i_stat.st_mode & S_IFDIR) == 0) {
        printf("Is not a DIR\n");
        return -1;
    }
    // 查找目录块
    int num = inode[ino].i_stat.st_blocks;
    if (num > 10) {
        printf("没有实现一级索引\n");
    }
    for (int l = 0; l < num; l++) {
        dir_t* d = &BLK[inode[ino].blk1[l]].dentry;
        //遍历ino 各个块查找name
        for (int i = 0; i < d->dir_list_size; i++) {
            if (mode & RECURSIVE) {
                //当前为递归查找模式
                if (inode[d->dir_list[i].d_ino].i_stat.st_mode & S_IFDIR && NOTDEFAULTDIR(d->dir_list[i].d_name)) {
                    return find_name_in_dir(d->dir_list[i].d_ino, name, mode);
                }
            }
            if (strcmp(d->dir_list[i].d_name, name) == 0) {
                //当处于递归查找时，返回文件所在目录ino号
                return mode & RECURSIVE ? ino : d->dir_list[i].d_ino;
            }
        }
    }
    return -1;
}

int creat_stat(const user_t* user, int ino, int mode) {
    inode[ino].i_stat.st_ino = ino;
    inode[ino].i_stat.st_mode = mode | S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP |S_IXGRP;
    inode[ino].i_stat.st_nlink = 1;
    inode[ino].i_stat.st_size = 0;
    inode[ino].i_stat.st_uid = user->pw_uid;
    inode[ino].i_stat.st_gid = user->pw_gid;
    inode[ino].i_stat.st_ctime = inode[ino].i_stat.st_atime =
      inode[ino].i_stat.st_mtime = time(NULL);
    inode[ino].i_stat.st_blksize = BLOCK_SIZE;
    inode[ino].i_stat.st_blocks = 0; // 创建时st_blocks默认设置为0
    return 0;
}

int creat_dirent(int fino, int ino, const char* name) {
    for (int i = 0; i < inode[fino].i_stat.st_blocks; i++) {
        int num = inode[fino].blk1[i];
        if (BLK[num].dentry.dir_list_size != MAX_DIRLIST) {
            int size = BLK[num].dentry.dir_list_size++;
            BLK[num].dentry.dir_list[size].d_ino = ino;
            strcpy(BLK[num].dentry.dir_list[size].d_name, name);
            return 0;
        }
    }
    //新分配一个块并将dir装入
    if (inode[fino].i_stat.st_blocks == 10) { // 目录存储已满
        printf("无法新增目录项\n");
        return -1;
    }
    int num;
    if ((num = add_blk_for_file(ino)) == -1) {
        return -1;
    }
    int size = BLK[num].dentry.dir_list_size++;
    BLK[num].dentry.dir_list[size].d_ino = ino;
    strcpy(BLK[num].dentry.dir_list[size].d_name,name);
    return 0;
}

// 就不加const是吧*3
int creat_dir(int ino, int f_ino, const char* dir_name) {
    //为dir分配内存
    if (add_blk_for_file(ino) == -1) {
        printf("无可用空间\n");
        return -1;
    }
    int num = inode[ino].blk1[0];
    dir_t* d = &BLK[num].dentry;
    //将名字保存在结构中
    strcpy(d->dir_name, dir_name);
    d->dir_list_size = 0;
    creat_dirent(ino, ino, ".");
    creat_dirent(ino, f_ino, "..");
    return 0;
}

int creat_ino() {
    for (int i = 0; i < INODE_NUM; i++) {
        if (inode[i].i_stat.st_blocks <= 0) {
        memset(&inode[i], 0, sizeof(inode_t));
        return i;
        }
    }
    return -1;
}

int creat(const user_t* user, int cwd, const char* args, int mode) {
    //检查当前用户在该文件夹内是否拥有写权限
    if (access(user, cwd, W_OK) != 0) {
        printf("mkdir: access error\n");
        return 0;
    }
    //查找inode空项，若inode数组已满则返回
    int ino = creat_ino();
    if (ino == -1) {
        printf("inode 数组已满\n");
        return -1;
    }
    // 设置文件stat相关信息
    if (creat_stat(user, ino, mode) == -1) {
        return -1;
    }
    //创建目录数据
    if (mode == S_IFDIR) {
        if (creat_dir(ino, cwd, args) == -1) {
        return -1;
        }
    } else if (mode == S_IFREG) {
        //分配普通文件的空间，初始分配一个块
        if (add_blk_for_file(ino) == -1) {
            printf("无可用空间\n");
            return -1;
        }
    }   
    //若不为根节点，在当前创建一个指向创建目录inode的条目
    if (ino != 0) {
        creat_dirent(cwd, ino, args);
    }
    return ino;
}

int mkdir(const user_t* user, int cwd, const char* dir_name, int mode) {
    (void)mode;
    //检查当前目录中是否存在同名文件
    if (strcmp(dir_name, "/") != 0 && find_name_in_dir(cwd, dir_name, 0) != -1) {
        printf("mkdir: cannot create directory '%s': File exists\n",dir_name);
        return -1;
    }
    //创建目录
    if (creat(user, cwd, dir_name,S_IFDIR) != 0){
        return -1;
    }
    return 0;
}

void ls(int ino, char* args) {
    if (args == NULL) {
        //遍历块
        for (int i = 0; i < inode[ino].i_stat.st_blocks; i++) {
            int num = inode[ino].blk1[i];
            dir_t* d = &BLK[num].dentry;
            for (int j = 0; j < d->dir_list_size; j++) {
                int _ino = d->dir_list[j].d_ino;
                printf("%c%c%c%c%c%c%c%c%c%c",HAVE_ACCESS(_ino,S_IFDIR,'d'),HAVE_ACCESS(_ino,S_IRUSR,'r'),\
                    HAVE_ACCESS(_ino,S_IWUSR,'w'),HAVE_ACCESS(_ino,S_IXUSR,'x'),HAVE_ACCESS(_ino,S_IRGRP,'r'),\
                    HAVE_ACCESS(_ino,S_IWGRP,'w'),HAVE_ACCESS(_ino,S_IXGRP,'x'),HAVE_ACCESS(_ino,S_IROTH,'r'),\
                    HAVE_ACCESS(_ino,S_IWOTH,'w'),HAVE_ACCESS(_ino,S_IXOTH,'x'));
                printf("%s ",ctime(&inode[_ino].i_stat.st_ctime));
                printf("%s\n",d->dir_list[j].d_name);
            }
        }
    }
}

void pwd(int cwd, char* buf, int ino) {
    if (ino == 0) {
        strcpy(buf, "/");
        return;
    }
    dir_t* tmp = &BLK[inode[ino].blk1[0]].dentry;
    pwd(cwd, buf, tmp->dir_list[1].d_ino);
    strcpy(buf+strlen(buf), tmp->dir_name);
    if(ino != cwd){
        strcpy(buf+strlen(buf), "/");
    }
    return ;
}

void cd(const user_t* user, int* cwd, const char* dir_name) {
    int ino;
    if ((ino = find_name_in_dir(*cwd, dir_name, 0)) == -1) { // 文件不存在
        printf("cd: %s: No such file or directory\n", dir_name);
        return;
    }
    if ((inode[ino].i_stat.st_mode & S_IFDIR) == 0) { // 选择的文件名不是目录
        printf("cd: %s: Not a directory\n", dir_name);
        return;
    }
    //当前用户对该目录没有读权限
    if (access(user, ino, R_OK) != 0) {
        printf("cd: access error\n");
        return ;
    }
    *cwd = ino;//工作目录转移
}

int rm_dir_item(dir_t* d, int n) {
    for (int i = n + 1;i < d->dir_list_size; i++) {
        d->dir_list[i - 1].d_ino = d->dir_list[i].d_ino;
        strcpy(d->dir_list[i - 1].d_name, d->dir_list[i].d_name);
    }
    d->dir_list_size--;
    return 0;
}

void rm(const user_t* user, int cwd, const char* args, int workIno, int mode) {
    int ino;
    // 文件不存在
    if ((ino = find_name_in_dir(workIno,args,0)) == -1) {
        printf("rmdir: %s: No such file or directory\n", args);
        return;
    }
    // 根目录不能删除
    if (ino == 0) {
        printf("can not remove root\n");
        return;
    }
    // 选择的文件名与mode不匹配
    if ((inode[ino].i_stat.st_mode & mode) == 0) {
        printf("rmdir: %s: Not a directory\n", args);
        return;
    }
    // 检查当前用户有没有执行权限
    if (access(user, cwd, W_OK) != 0) {
        printf("rmdir: access error\n");
        return ;
    }
    if (--inode[ino].i_stat.st_nlink == 0) {
        // 要删除的是文件夹
        if (mode & S_IFDIR) {
            dir_t* d2 = &BLK[inode[ino].blk1[0]].dentry;
            if (mode & RECURSIVE) {
                // 直接开始递归删除
                int num = inode[ino].i_stat.st_blocks;
                for (int l = 0; l < num; l++) {
                    dir_t* d = &BLK[inode[ino].blk1[l]].dentry;
                    // 遍历ino 各个块查找name
                    for (int i = 0; i < d->dir_list_size; i++) {
                        if (strcmp(d->dir_list[i].d_name,".") == 0 || strcmp(d->dir_list[i].d_name,"..") == 0)
                            continue;
                        if (inode[d->dir_list[i].d_ino].i_stat.st_mode & S_IFDIR) {
                        // 当前待删除的是目录，递归删除
                            rm(user, cwd, d->dir_list[i].d_name, ino, mode); // 进入到目录中删除其文件
                        } else {
                            rm(user, cwd, d->dir_list[i].d_name, ino, S_IFREG);
                        }
                    }
                }
            } else if (d2->dir_list_size != 2 || inode[ino].i_stat.st_blocks != 1) {
                //检查当前文件夹是否为空，若不为空则不能删除
                printf("rmdir: Directory not empty\n");
                return ;
            }
        }
        // 释放所有blk,有一级索引则先删除一级索引
        int blockNum = inode[ino].i_stat.st_blocks;
        if (blockNum > 10) {
            for (int i = 0; i < blockNum - 10; i++) {
                CLEAR_BLK_FLAG(BLK[inode[ino].blk2].index[i]);
            }
        }
        for (int i = 0; i < 10; i++) {
            if (inode[ino].blk1[i] != 0) {
                CLEAR_BLK_FLAG(inode[ino].blk1[i]);
                inode[ino].blk1[i] = 0;
            }
        }
        //将stat信息清空
        memset(&inode[ino].i_stat, 0, sizeof(stat_t));
    }
    //从目录中一处待删除目录项
    for (int i = 0; i < inode[cwd].i_stat.st_blocks; i++) {
        dir_t* d = &GET_BLKN(cwd, i).dentry;
        for (int j = 0; j < d->dir_list_size; j++) {
            if (d->dir_list[j].d_ino == ino) {
                rm_dir_item(d, j);
            }
        }
    }
    return ;
}

void rmdir(const user_t* user, int cwd, char* dir_name, int mode) {
    if (mode & RECURSIVE) {
        rm(user, cwd, dir_name, cwd, S_IFDIR | RECURSIVE);
        return ;
    } else {
        //递归删除整个目录
        rm(user, cwd, dir_name, cwd, S_IFDIR);
        return ;
    }
}

void creat_file(const user_t* user, int cwd, const char* args) {
    //检查当前目录中是否存在同名文件
    if (find_name_in_dir(cwd, args, 0) != -1) {
        printf("creat: cannot create file '%s': File exists\n", args);
        return ;
    }
    creat(user, cwd, args, S_IFREG);
    return ;
}

int open_file(const user_t* user, int cwd, const char* args) {
    // 首先查看用户对该目录是否有读权限
    if (access(user, cwd, R_OK) != 0) {
        printf("write: access r error\n");
        return -1;
    }
    // 首先查找文件是否存在
    int ino; // 文件的ino
    if ((ino = find_name_in_dir(cwd, args, 0)) == -1) {
        printf("no file in dir\n");
        return -1;
    }
    // 查看文件是否为reg类型
    if ((inode[ino].i_stat.st_mode & S_IFREG) == 0) {
        printf("%s: Not a regfile\n", args);
        return -1;
    }
    return ino;
}

int read_file(const user_t* user, int cwd, int argc, const char* argv[]) {
    (void)argc;
    //打开文件
    int ino = open_file(user, cwd, argv[0]);
    if (ino == -1) {
        return -1;
    }
    int start = atoi(argv[1]);
    int len = atoi(argv[2]);
    while (len--) {
        //通过起始数得出需要访问的起始页
        int page = start / BLOCK_SIZE;
        if (start >= inode[ino].i_stat.st_size || GET_CHAR(ino,page,start - page * BLOCK_SIZE) == '\0') {
            printf("%c", '.');
        } else {
            printf("%c", GET_CHAR(ino, page, start - page * BLOCK_SIZE));
        }
        start++;
    }
    printf("\n");
    inode[ino].i_stat.st_atime = time(NULL);
    return 0;
}

int write_file(const user_t* user, int cwd, int argc, const char* argv[]) {
    (void)argc;
    int ino = open_file(user, cwd, argv[0]);
    if (ino == -1) {
        return -1;
    }
    // 查看文件写权限
    if (access(user, ino, W_OK) != 0) {
        printf("write: access w error\n");
        return -1;
    }
    // 检查文件空间是否够用
    int start = atoi(argv[1]);
    int len = atoi(argv[2]);
    int page = (start + len) / BLOCK_SIZE + 1;
    // 为文件分配空间
    while (inode[ino].i_stat.st_blocks < page) {
        if (add_blk_for_file(ino) == -1) {
            printf("存储空间已满");
            return -1;
        }
    }
    // 更改文件长度
    if (start + len > inode[ino].i_stat.st_size) {
        inode[ino].i_stat.st_size = start + len;
    }
    char* newdata = readline("input:");
    // 写入文件
    int i = 0;
    while (len--) {
        // 通过起始数得出需要访问的起始页
        page = start / BLOCK_SIZE;
        GET_CHAR(ino,page,start - page * BLOCK_SIZE) = newdata[i++];
        start++;
    }
    free(newdata);
    // 更改stat中的时间
    inode[ino].i_stat.st_atime = inode[ino].i_stat.st_mtime = time(NULL);
    return 0;
}

int link_file(int cwd, int argc, const char* argv[]) {
    (void)argc;
    // 检查当前目录中是否存在同名文件
    if (find_name_in_dir(cwd, argv[1], 0) != -1) {
        printf("creat: cannot create file '%s': File exists", argv[1]);
        return -1;
    }
    // 首先查找文件是否存在
    int ino; // 文件的ino
    if ((ino = find_name_in_dir(cwd, argv[0], 0)) == -1) {
        printf("no file in dir\n");
        return -1;
    }
    // 添加一个目录项
    if (creat_dirent(cwd, ino, argv[1]) != -1) {
        // 为ino添加一个硬链接
        inode[ino].i_stat.st_nlink++;
        return 0;
    }
    return -1;
}

void find_file(int cwd, const char* args,int ino) {
    int num = inode[ino].i_stat.st_blocks;
    for (int l = 0; l < num; l++) {
        dir_t* d = &BLK[inode[ino].blk1[l]].dentry;
        //遍历ino 各个块查找name
        for (int i = 0; i < d->dir_list_size; i++) {
            if (inode[d->dir_list[i].d_ino].i_stat.st_mode & S_IFDIR &&
                NOTDEFAULTDIR(d->dir_list[i].d_name)) {
                find_file(cwd, args, d->dir_list[i].d_ino);
            }
            if (inode[d->dir_list[i].d_ino].i_stat.st_mode & S_IFREG && strcmp(d->dir_list[i].d_name,args) == 0) {
                char str[100];
                pwd(cwd, str, ino);
                printf("%s%s\n",str,args);
            }
        }
    }
}
