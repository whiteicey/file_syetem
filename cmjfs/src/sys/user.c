/**
 * @file user.c
 * @version 0.1
 * @date 2021-11-14
 */

#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/user.h>

int idle_uid = 101;

// TODO: NOTE： ?
// 别忘了放好passwd文件！

// 这个确实是不能const，指user
int login(user_t* user, const char* args) {
    if (args == NULL) {
        printf("login:");
        scanf("%s", user->pw_name);
    } else {
        strcpy(user->pw_name, args);
    }
    //打开存放用户数据的文件passwd
    FILE* fp;
    if ((fp = fopen("passwd", "r")) == NULL) {
        printf("No user data\n");
        return -1;
    }
    if (fseek(fp, 0, SEEK_SET) != 0) {
        printf("fseek error\n");
        fclose(fp);
        return -1;
    }
    char tmp[100];
    while (fgets(tmp, 100, fp) != NULL) {
        char* p = strtok(tmp, ":");
        if (strcmp(user->pw_name, p) == 0) {
            printf("passwd:");
            scanf("%s", user->pw_passwd);
            p = strtok(NULL, ":");
            if (strcmp(user->pw_passwd, p) == 0) { // 登录成功
                p = strtok(NULL, ":");
                user->pw_uid = atoi(p);
                p = strtok(NULL, ":");
                user->pw_gid = atoi(p);
                fclose(fp);
                return 0;
            } else {
                printf("passwd error\n");
                fclose(fp);
                return -1;
            }
        }
    }
    printf("no user\n");
    fclose(fp);
    return -1;
}

// TODO: 改为外部管理器形式
int find_user(const char* args) {
    if (*args == '\0') {
        return -1;
    }
    FILE* fp;
    if ((fp = fopen("passwd", "r")) == NULL) {
        printf("No user data\n");
        return -1;
    }
    char tmp[100];
    while (fgets(tmp, 100, fp) != NULL) {
        char* p = strtok(tmp, ":");
        if (strcmp(args, p) == 0) {
            fclose(fp);
            return 0;
        }
    }
    fclose(fp);
    return -1;
}

// NOTE：请保持命名风格一致
void add_user(const user_t* user, const char* args) {
    if (idle_uid > 198) {
        printf("用户数量已达到最大\n");
        return ;
    }
    //当前用户必须为root
    if (user->pw_uid != 0) {
        printf("权限不足\n");
        return ;
    }
    //打开文件passwd
    FILE* fp;
    if ((fp = fopen("passwd", "r+")) == NULL) {
        printf("No user data\n");
        return ;
    }
    //搜索文件查找用户是否重复
    if (find_user(args) == 0) {
        printf("Username already exists\n");
        return ;
    }
    //在末尾添加用户
    if (fseek(fp, 0, SEEK_END) != 0) {
        printf("fseek error\n");
        fclose(fp);
        return ;
    }
    printf("passwd>");
    char newpasswd[20];
    scanf("%s", newpasswd);
    fputs(args, fp);
    fputs(":", fp);
    fputs(newpasswd, fp);
    fputs(":", fp);
    sprintf(newpasswd, "%d", idle_uid);
    fputs(newpasswd, fp);
    fputs(":", fp);
    fputs("100\n", fp);
    fclose(fp);
}
