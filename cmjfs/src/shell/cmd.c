/**
 * @file command.h
 * @version 0.1
 * @date 2021-11-14
 */

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <shell/shell.h>
#include <shell/cmd.h>
#include <sys/fs/fsops.h>

// 指令表细节
cmd_t cmd_table[] = {
  { "ls",      "show catalog items",     cmd_ls      }, // 1
  { "pwd",     "show current directory", cmd_pwd     }, // 2
  { "cd",      "enter a directory",      cmd_cd      }, // 3
  { "rmdir",   "remove a directory",     cmd_rmdir   }, // 4
  { "mkdir",   "creat a directory",      cmd_mkdir   }, // 5
  { "su",      "change users",           cmd_su      }, // 6
  { "whoami",  "show current user name", cmd_whoami  }, // 7
  { "useradd", "add new user",           cmd_useradd }, // 8
  { "creat",   "creat a new file",       cmd_creat   }, // 9
  { "read",    "read a file",            cmd_read    }, // 10
  { "write",   "write a file",           cmd_write   }, // 11
  { "rm",      "remove a file",          cmd_rm      }, // 12
  { "ln",      "hard link",              cmd_ln      }, // 13
  { "find",    "find a file",            cmd_find    }  // 14
};

void cmd_pwd(char* args) {
    // 这一行是为了压制未引用变量的警告
    (void)args;
    char str[100];
    pwd(work_dir, str, work_dir);
    printf("%s\n",str);
}

// NOTE：接口相比参数凑数，可能有更好的方案，但我还没研究明白（
void cmd_ls(char* args) {
    (void)args;
    ls(work_dir, NULL);
}

void cmd_mkdir(char* args) {
    // 没有输入参数,返回
    if (*args == '\0') {
        printf("mkdir: missing operand\n");
        return ;
    }
    int n = 0;
    char* argv[5];
    char* p = strtok(NULL, " ");
    while (p) {
        argv[n++] = p;
        p = strtok(NULL, " ");
    }
    mkdir(&cnt_user, work_dir, argv[0], 0);
}

void cmd_cd(char* args) {
    if (*args == '\0') {
       return ;
    }
    int n = 0;
    // NOTE：说真的，你真的要设置这么小的buffer？
    char* argv[5];
    char* p = strtok(NULL, " ");
    while (p) {
        argv[n++] = p;
        p = strtok(NULL, " ");
    }
    cd(&cnt_user, &work_dir, argv[0]);
}

void cmd_rmdir(char* args) {
    if (*args == '\0') {
        printf("rmdir: missing operand\n");
        return ;
    }
    int n = 0;
    char* argv[5];
    char* p = strtok(NULL, " ");
    while (p) {
        argv[n++] = p;
        p = strtok(NULL, " ");
    }
    if (strcmp(argv[0], "-r") == 0 && n == 2) {
        //递归删除整个目录
        rmdir(&cnt_user, work_dir, argv[1], RECURSIVE);
    } else {
        rmdir(&cnt_user, work_dir, argv[0], 0);
    }
}

// NOTE：讲真，真实的su指令好像不是干这个的？
void cmd_su(char* args) {
    login(&cnt_user, args);
}

void cmd_whoami(char* args) {
    (void)args;
    printf("%s\n", cnt_user.pw_name);
    return ;
}

void cmd_useradd(char* args) {
    //判断args是否为NULL
    if (*args == '\0') {
        printf("useradd: No args\n");
        return ;
    }
    int n = 0;
    char* argv[5];
    char* p = strtok(NULL, " ");
    while (p) {
        argv[n++] = p;
        p = strtok(NULL, " ");
    }
    add_user(&cnt_user, argv[0]);
}

// NOTE：实际系统中touch指令可以做这件事。
// 不同的是，touch实际上会更新文件状态，对已存在的文件亦然。
void cmd_creat(char* args) {
    //检查args是否为空
    if (*args == '\0') {
        printf("creat: No args\n");
        return ;
    }
    int n = 0;
    char* argv[5];
    char* p = strtok(NULL, " ");
    while (p) {
        argv[n++] = p;
        p = strtok(NULL, " ");
    }
    creat_file(&cnt_user, work_dir, argv[0]);
}

// NOTE：你有这功夫复制粘贴，就不能写个新函数吗？？
void cmd_rm(char* args) {
    if (*args == '\0') {
        printf("No args\n");
        return ;
    }
    int n = 0;
    char* argv[5];
    char* p = strtok(NULL, " ");
    while (p) {
        argv[n++] = p;
        p = strtok(NULL, " ");
    }
    rm(&cnt_user, work_dir, argv[0], work_dir, S_IFREG);
}

void cmd_read(char* args) {
    if (*args == '\0') {
        printf("No args\n");
        return ;
    }
    int n = 0;
    char* argv[5];
    char* p = strtok(NULL, " ");
    while (p) {
        argv[n++] = p;
        p = strtok(NULL, " ");
    }
    if (n != 3) {
        printf("args error:read [offset] [len]\n");
        return;
    }
    read_file(&cnt_user, work_dir, n, argv);
}

void cmd_write(char* args){
    if(*args == '\0') {
        printf("No args\n");
        return ;
    }
    int n = 0;
    char* argv[5];
    char* p = strtok(NULL," ");
    while(p) {
        argv[n++] = p;
        p = strtok(NULL," ");
    }
    if(n > 3) {
        printf("args error:write [offset] [len]\n");
        return;
    }
    write_file(&cnt_user, work_dir, n, argv);
}

void cmd_ln(char* args) {
    if (*args == '\0') {
        printf("No args\n");
        return ;
    }
    int n = 0;
    // int i = 0;
    char* argv[5];
    char* p = strtok(NULL, " ");
    while (p) {
        argv[n++] = p;
        p = strtok(NULL, " ");
    }
    link_file(work_dir, n, argv);
}

void cmd_find(char* args) {
    if(*args == '\0') {
        printf("No args\n");
        return ;
    }
    int n = 0;
    // int i = 0;
    char* argv[5];
    char* p = strtok(NULL," ");
    while(p) {
        argv[n++] = p;
        p = strtok(NULL," ");
    }
    find_file(work_dir, argv[0], work_dir);
}
