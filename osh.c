#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_LINE 80 //最长命令行数
#define MAX_HISTORY 10 //最多访问历史数

char *history[MAX_HISTORY]; //命令历史
int hist_count = 0;
int hist_next = 0;

void add_history(const char *cmd) {//添加命令历史
    if (hist_count < MAX_HISTORY) {
        history[hist_count] = strdup(cmd);
        hist_count++;
    } else {
        free(history[hist_next]);
        history[hist_next] = strdup(cmd);
        hist_next = (hist_next + 1) % MAX_HISTORY;
        hist_count++;
    }
}

char *get_history(int n) {//从历史中检索命令
    if (n <= 0 || n > hist_count) return NULL;
    int total = hist_count < MAX_HISTORY ? hist_count : MAX_HISTORY;
    int oldest_index = (hist_count < MAX_HISTORY) ? 0 : hist_next;
    int offset = n - (hist_count - total) - 1;
    int idx = (oldest_index + offset) % MAX_HISTORY;
    return history[idx];
}

void show_history() { //执行输出
    int total = hist_count < MAX_HISTORY ? hist_count : MAX_HISTORY;
    int start = (hist_count < MAX_HISTORY) ? 0 : hist_next;
    for (int i = 0; i < total; i++) {
        int idx = (start + i) % MAX_HISTORY;
        int cmd_num = hist_count - total + i + 1;
        printf("%d %s\n", cmd_num, history[idx]);
    }
}

int parse_command(char *input, char *args[], int *bg) { //命令解析
    char *token;
    int i = 0;
    token = strtok(input, " \t\n");
    while (token != NULL && i < MAX_LINE/2) {
        args[i++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[i] = NULL;
    *bg = 0;
    if (i > 0 && strcmp(args[i-1], "&") == 0) {
        args[i-1] = NULL;
        *bg = 1;
    }
    return i;
}

int main(void) {
    char *args[MAX_LINE/2 + 1];
    char input[MAX_LINE];
    int should_run = 1;
    int bg;

    while (should_run) {
        printf("osh> ");
        fflush(stdout);

        if (fgets(input, MAX_LINE, stdin) == NULL)
            break;
        input[strcspn(input, "\n")] = '\0';
        if (strlen(input) == 0) continue;

        // 处理内置命令
        if (strcmp(input, "exit") == 0) {
            should_run = 0;
            continue;
        }
        if (strcmp(input, "history") == 0) {
            show_history();
            continue;
        }

        // 历史展开
        char cmd_buf[MAX_LINE];
        if (strcmp(input, "!!") == 0) {
            if (hist_count == 0) {
                printf("No commands in history.\n");
                continue;
            }
            char *last = get_history(hist_count);
            printf("%s\n", last);
            strcpy(cmd_buf, last);
        } else if (input[0] == '!') {
            int n;
            if (sscanf(input, "!%d", &n) == 1) {
                if (hist_count == 0) {
                    printf("No commands in history.\n");
                    continue;
                }
                char *cmd = get_history(n);
                if (cmd == NULL) {
                    printf("No such command in history.\n");
                    continue;
                }
                printf("%s\n", cmd);
                strcpy(cmd_buf, cmd);
            } else {
                printf("Invalid history command.\n");
                continue;
            }
        } else {
            strcpy(cmd_buf, input);
        }

        // 将实际执行的命令加入历史（排除历史命令本身）
        add_history(cmd_buf);

        // 解析命令
        parse_command(cmd_buf, args, &bg);

        if (args[0] == NULL) continue; // 空命令

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            continue;
        } else if (pid == 0) {
            // 子进程执行命令
            if (execvp(args[0], args) == -1) {
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        } else {
            // 父进程：若前台则等待
            if (!bg) {
                wait(NULL);
            } else {
                printf("[%d] running in background\n", pid);
            }
        }
    }

    // 清理历史内存
    for (int i = 0; i < MAX_HISTORY && i < hist_count; i++) {
        free(history[i]);
    }
    return 0;
}
