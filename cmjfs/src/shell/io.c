/**
 * @file io.c
 * @version 0.1
 * @date 2021-11-14
 */

#include <shell/io.h>

char* readline(const char* str) {
    printf("%s", str);
    // 你对getchar的理解很有可能是错误的。但我们暂时忽略这点。
    char ch = (char)getchar();
    if (ch != '\n') ungetc(ch, stdin);
    // NOTE：事实上，sizeof(char)应该永远是1。大概
    // NOTE：我们非常不建议用fgets，也不建议直接操作流。存在性能问题和并发问题
    char* p = (char*) malloc(sizeof(char) * 128);
    fgets(p, 128, stdin);
    const int len = (int)strlen(p);
    if (p[len - 1] == '\n') p[len - 1] = '\0';
    // 过长时的进一步处理
    return p;
}

void add_history(char* p) {
    (void)p;

    return ;
}//读取指令历史，未完成

char* rl_gets() {
    static char *line_read = NULL;
    // 释放存储区
    // malloc存在巨大的性能问题，应当减少malloc。
    if (line_read) {
        free(line_read);
        line_read = NULL;
    }
    line_read = readline("shell>");
    if (line_read && *line_read) {
        add_history(line_read);
    }
    return line_read;
}
