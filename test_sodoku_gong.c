#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    const char *name;           // 测试用例名称
    const char *input;          // 9行数独数据（空格分隔）
    const char *expected;       // 预期输出："宫没有问题。" 或 "不符合要求，宫有问题。"
} TestCase;

// 测试用例集（共12个，覆盖各种错误类型）
TestCase test_cases[] = {
    // 1. 完全正确的数独（所有宫合法）
    {
        "完全正确的数独",
        "5 3 4 6 7 8 9 1 2\n"
        "6 7 2 1 9 5 3 4 8\n"
        "1 9 8 3 4 2 5 6 7\n"
        "8 5 9 7 6 1 4 2 3\n"
        "4 2 6 8 5 3 7 9 1\n"
        "7 1 3 9 2 4 8 5 6\n"
        "9 6 1 5 3 7 2 8 4\n"
        "2 8 7 4 1 9 6 3 5\n"
        "3 4 5 2 8 6 1 7 9\n",
        "宫没有问题。"
    },
    // 2. 宫重复（左上角宫两个1）
    {
        "宫重复（左上角宫）",
        "1 2 3 4 5 6 7 8 9\n"
        "1 5 6 7 8 9 2 3 4\n"
        "7 8 9 1 2 3 4 5 6\n"
        "4 5 6 2 3 1 8 9 7\n"
        "8 9 7 5 6 4 1 2 3\n"
        "2 3 1 8 9 7 5 6 4\n"
        "3 1 4 6 7 2 9 8 5\n"
        "5 6 2 9 1 8 3 7 4\n"
        "9 7 8 3 4 5 6 1 2\n",
        "不符合要求，宫有问题。"
    },
    // 3. 宫重复（中心宫两个5）
    {
        "宫重复（中心宫）",
        "5 3 4 6 7 8 9 1 2\n"
        "6 7 2 1 9 5 3 4 8\n"
        "1 9 8 3 4 2 5 6 7\n"
        "8 5 9 7 6 1 4 2 3\n"
        "4 2 5 8 5 3 7 9 1\n"   // 中心宫两个5
        "7 1 3 9 2 4 8 5 6\n"
        "9 6 1 5 3 7 2 8 4\n"
        "2 8 7 4 1 9 6 3 5\n"
        "3 4 5 2 8 6 1 7 9\n",
        "不符合要求，宫有问题。"
    },
    // 4. 宫重复（右下角宫两个9）
    {
        "宫重复（右下角宫）",
        "5 3 4 6 7 8 9 1 2\n"
        "6 7 2 1 9 5 3 4 8\n"
        "1 9 8 3 4 2 5 6 7\n"
        "8 5 9 7 6 1 4 2 3\n"
        "4 2 6 8 5 3 7 9 1\n"
        "7 1 3 9 2 4 8 5 6\n"
        "9 6 1 5 3 7 9 8 4\n"   // 右下角宫两个9
        "2 8 7 4 1 9 6 3 5\n"
        "3 4 5 2 8 6 1 7 9\n",
        "不符合要求，宫有问题。"
    },
    // 5. 数值超范围（出现0，中心宫）
    {
        "数值超范围（0）",
        "5 3 4 6 7 8 9 1 2\n"
        "6 7 2 1 9 5 3 4 8\n"
        "1 9 8 3 4 2 5 6 7\n"
        "8 5 9 7 6 1 4 2 3\n"
        "4 2 6 0 5 3 7 9 1\n"
        "7 1 3 9 2 4 8 5 6\n"
        "9 6 1 5 3 7 2 8 4\n"
        "2 8 7 4 1 9 6 3 5\n"
        "3 4 5 2 8 6 1 7 9\n",
        "不符合要求，宫有问题。"
    },
    // 6. 数值超范围（出现10，中宫）
    {
        "数值超范围（10）",
        "5 3 4 6 7 8 9 1 2\n"
        "6 7 2 1 9 5 3 4 8\n"
        "1 9 8 3 4 2 5 6 7\n"
        "8 5 9 7 6 1 4 2 3\n"
        "4 2 6 8 10 3 7 9 1\n"
        "7 1 3 9 2 4 8 5 6\n"
        "9 6 1 5 3 7 2 8 4\n"
        "2 8 7 4 1 9 6 3 5\n"
        "3 4 5 2 8 6 1 7 9\n",
        "不符合要求，宫有问题。"
    },
    // 7. 行重复（第一行两个5）
    {
        "行重复（第一行两个5）",
        "5 3 4 6 5 8 9 1 2\n"
        "6 7 2 1 9 5 3 4 8\n"
        "1 9 8 3 4 2 5 6 7\n"
        "8 5 9 7 6 1 4 2 3\n"
        "4 2 6 8 5 3 7 9 1\n"
        "7 1 3 9 2 4 8 5 6\n"
        "9 6 1 5 3 7 2 8 4\n"
        "2 8 7 4 1 9 6 3 5\n"
        "3 4 5 2 8 6 1 7 9\n",
        "不符合要求，宫有问题" 
    },
    // 8. 列重复
    {
        "列重复（第一列两个5）",
        "5 3 4 6 7 8 9 1 2\n"
        "5 7 2 1 9 5 3 4 8\n"
        "1 9 8 3 4 2 5 6 7\n"
        "8 5 9 7 6 1 4 2 3\n"
        "4 2 6 8 5 3 7 9 1\n"
        "7 1 3 9 2 4 8 5 6\n"
        "9 6 1 5 3 7 2 8 4\n"
        "2 8 7 4 1 9 6 3 5\n"
        "3 4 5 2 8 6 1 7 9\n",
        "不符合要求，宫有问题。"
    },
    // 9. 行重复导致宫重复（同一宫内重复）
    {
        "行重复导致宫重复（左上角宫两个1）",
        "1 1 3 4 5 6 7 8 9\n"   // 第一行前两个1，左上角宫内重复
        "6 7 2 1 9 5 3 4 8\n"
        "1 9 8 3 4 2 5 6 7\n"
        "8 5 9 7 6 1 4 2 3\n"
        "4 2 6 8 5 3 7 9 1\n"
        "7 1 3 9 2 4 8 5 6\n"
        "9 6 1 5 3 7 2 8 4\n"
        "2 8 7 4 1 9 6 3 5\n"
        "3 4 5 2 8 6 1 7 9\n",
        "不符合要求，宫有问题。"
    },
    // 10. 全1数独（所有宫重复）
    {
        "全1数独",
        "1 1 1 1 1 1 1 1 1\n"
        "1 1 1 1 1 1 1 1 1\n"
        "1 1 1 1 1 1 1 1 1\n"
        "1 1 1 1 1 1 1 1 1\n"
        "1 1 1 1 1 1 1 1 1\n"
        "1 1 1 1 1 1 1 1 1\n"
        "1 1 1 1 1 1 1 1 1\n"
        "1 1 1 1 1 1 1 1 1\n"
        "1 1 1 1 1 1 1 1 1\n",
        "不符合要求，宫有问题。"
    },
    // 11. 全9数独（所有宫重复）
    {
        "全9数独",
        "9 9 9 9 9 9 9 9 9\n"
        "9 9 9 9 9 9 9 9 9\n"
        "9 9 9 9 9 9 9 9 9\n"
        "9 9 9 9 9 9 9 9 9\n"
        "9 9 9 9 9 9 9 9 9\n"
        "9 9 9 9 9 9 9 9 9\n"
        "9 9 9 9 9 9 9 9 9\n"
        "9 9 9 9 9 9 9 9 9\n"
        "9 9 9 9 9 9 9 9 9\n",
        "不符合要求，宫有问题。"
    },
    // 12. 合法数独变体（宫正确）
    {
        "合法数独变体",
        "2 7 6 3 1 4 9 5 8\n"
        "8 5 4 9 6 2 7 3 1\n"
        "3 1 9 8 7 5 2 4 6\n"
        "6 9 5 4 2 8 1 7 3\n"
        "7 4 2 1 5 3 6 8 9\n"
        "1 8 3 7 9 6 5 2 4\n"
        "4 6 7 5 3 9 8 1 2\n"
        "9 2 8 6 4 1 3 5 7\n"
        "5 3 1 2 8 7 4 6 9\n",
        "宫没有问题。"
    }
};

// 运行原程序，返回输出（去除换行符）
char* run_sudoku_gong(const char *input) {
    char temp_in[] = "/tmp/sudoku_in_XXXXXX";
    int fd = mkstemp(temp_in);
    if (fd == -1) return NULL;
    FILE *f = fdopen(fd, "w");
    if (!f) { close(fd); unlink(temp_in); return NULL; }
    fprintf(f, "%s", input);
    fclose(f);

    char command[512];
    snprintf(command, sizeof(command), "./sudoku_gong < %s 2>/dev/null", temp_in);
    FILE *fp = popen(command, "r");
    if (!fp) { unlink(temp_in); return NULL; }

    char line[256];
    char *last_line = NULL;
    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == '\\n') continue;
        free(last_line);
        last_line = strdup(line);
    }
    pclose(fp);
    unlink(temp_in);

    if (last_line) {
        char *p = strchr(last_line, '\\n');
        if (p) *p = '\\0';
    }
    return last_line;
}

int main() {
    if (access("./sudoku_gong", X_OK) != 0) {
        fprintf(stderr, "错误：未找到可执行文件 ./sudoku_gong\\n");
        fprintf(stderr, "请先编译原程序：gcc -pthread sudoku_gong.c -o sudoku_gong\\n");
        return 1;
    }

    int total = sizeof(test_cases) / sizeof(TestCase);
    int passed = 0;

    printf("===== 数独宫检查程序测试 =====\\n");
    printf("原程序仅检查9个3x3宫及数值范围（1-9）\\n\\n");

    for (int i = 0; i < total; i++) {
        TestCase *tc = &test_cases[i];
        printf("测试[%d]: %s\\n", i+1, tc->name);
        char *output = run_sudoku_gong(tc->input);
        if (!output) {
            printf("  运行失败\\n");
        } else {
            if (strcmp(output, tc->expected) == 0) {
                printf("  通过 (输出: %s)\\n", output);
                passed++;
            } else {
                printf("  失败\\n");
                printf("    期望: %s\\n", tc->expected);
                printf("    实际: %s\\n", output);
            }
            free(output);
        }
    }

    printf("\\n总计: %d 个测试, %d 通过, %d 失败\\n", total, passed, total - passed);
    return (passed == total) ? 0 : 1;
}
