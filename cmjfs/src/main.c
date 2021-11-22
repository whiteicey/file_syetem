/**
 * @file main.c
 * @version 0.1
 * @date 2021-11-14
 */

// 压制C4996警告：strtok不安全
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <shell/shell.h>
#include <shell/cmd.h>
#include <shell/io.h>
#include <sys/fs/fsops.h>

int main() {
    printf("file system is running\n");
    // 创建根节点
    work_dir = 0;
    if (mkdir(&cnt_user, work_dir, "/", 0) != 0) {
        printf("cannot creat a root\n");
        return 0;
    }
    // 根目录创建成功，开始登陆
    while (login(&cnt_user, NULL) != 0);
    while (1) {
        char* str = rl_gets();
        char* cmd = strtok(str, " ");
        if (cmd == NULL) continue;
        int i;
        int cmd_matched = 0;
        for (i = 0; i < CMD_N; i++) {
            if (strcmp(cmd, cmd_table[i].name) == 0) { // 找到命令
                char* args = cmd + strlen(cmd) + 1;
                cmd_table[i].handler(args);
                cmd_matched = 1;
                break;
            }
        }
        if (!cmd_matched) {
            if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "exit") == 0
              || strcmp(cmd, "logout")) break;
            printf("Unknown command\n");
        }
    }
}
