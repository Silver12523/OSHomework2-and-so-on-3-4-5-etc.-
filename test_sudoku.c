#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    const char *name;           // 测试用例名称
    const char *input;          // 9行数独数据（空格分隔）
    int exp_gong_ok;            // 期望宫全部正确（1:正确,0:有误）
    int exp_row_ok;             // 期望行全部正确
    int exp_col_ok;             // 期望列全部正确
    int exp_final_ok;           // 期望最终数独正确
} TestCase;

// 标准正确的数独（用于构造合法用例）
static const char* VALID_SUDOKU =
    "5 3 4 6 7 8 9 1 2\n"
    "6 7 2 1 9 5 3 4 8\n"
    "1 9 8 3 4 2 5 6 7\n"
    "8 5 9 7 6 1 4 2 3\n"
    "4 2 6 8 5 3 7 9 1\n"
    "7 1 3 9 2 4 8 5 6\n"
    "9 6 1 5 3 7 2 8 4\n"
    "2 8 7 4 1 9 6 3 5\n"
    "3 4 5 2 8 6 1 7 9\n";

// 测试用例集
TestCase test_cases[] = {
    // 1. 完全正确的数独
    {
        "完全正确的数独",
        VALID_SUDOKU,
        1, 1, 1, 1
    },

    // 2. 行重复
    {
        "行重复",
        "5 3 4 6 5 8 9 1 2\n"
        "6 7 2 1 9 5 3 4 8\n"
        "1 9 8 3 4 2 5 6 7\n"
        "8 5 9 7 6 1 4 2 3\n"
        "4 2 6 8 5 3 7 9 1\n"
        "7 1 3 9 2 4 8 5 6\n"
        "9 6 1 5 3 7 2 8 4\n"
        "2 8 7 4 1 9 6 3 5\n"
        "3 4 5 2 8 6 1 7 9\n",
        0, 0, 0, 0
    },

    // 3. 列重复
    {
        "列重复（第九列两个9）",
        "5 3 4 6 7 8 9 1 2\n"
        "6 7 2 1 9 5 3 4 8\n"
        "1 9 8 3 4 2 5 6 7\n"
        "8 5 9 7 6 1 4 2 3\n"
        "4 2 6 8 5 3 7 9 1\n"
        "7 1 3 9 2 4 8 5 6\n"
        "9 6 1 5 3 7 2 8 4\n"
        "2 8 7 4 1 5 6 3 9\n" 
        "3 4 5 2 8 6 1 7 9\n",
        0, 1, 0, 0
    },

    // 4. 宫重复（左上角宫两个1）
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
        0, 1, 0, 0
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
        0, 0, 0, 0
    },

    // 6. 数值超范围（出现10，左中宫）
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
        0, 0, 0, 0
    },

    // 7. 行重复 + 宫重复+列重复
    {
        "行重复 + 宫重复+列重复",
        "1 1 3 4 5 6 7 8 9\n"
        "1 5 6 7 8 9 2 3 4\n"
        "7 8 9 1 2 3 4 5 6\n"
        "4 5 6 2 3 1 8 9 7\n"
        "8 9 7 5 6 4 1 2 3\n"
        "2 3 1 8 9 7 5 6 4\n"
        "3 1 4 6 7 2 9 8 5\n"
        "5 6 2 9 1 8 3 7 4\n"
        "9 7 8 3 4 5 6 1 2\n",
        0, 0, 0, 0
    },

    // 8. 列重复 + 宫重复（第一列两个1且左上角宫重复）
    {
        "列重复 + 宫重复",
        "1 2 3 4 5 6 7 8 9\n"
        "1 5 6 7 8 9 2 3 4\n"
        "7 8 9 1 2 3 4 5 6\n"
        "4 5 6 2 3 1 8 9 7\n"
        "8 9 7 5 6 4 1 2 3\n"
        "2 3 1 8 9 7 5 6 4\n"
        "3 1 4 6 7 2 9 8 5\n"
        "5 6 2 9 1 8 3 7 4\n"
        "9 7 8 3 4 5 6 1 2\n",
        0, 1, 0, 0
    },

    // 9. 全1数独（所有错误）
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
        0, 0, 0, 0
    },

    // 10. 宫错误
    {
        "宫错误（右上角宫）",
        "5 3 4 6 7 8 7 1 2\n"
        "6 7 2 1 9 5 3 4 8\n"
        "1 9 8 3 4 2 5 6 7\n"
        "8 5 9 7 6 1 4 2 3\n"
        "4 2 6 8 5 3 7 9 1\n"
        "7 1 3 9 2 4 8 5 6\n"
        "9 6 1 5 3 7 2 8 4\n"
        "2 8 7 4 1 9 6 3 5\n"
        "3 4 5 2 8 6 1 7 9\n",
        0, 0, 0, 0
    },

    // 11. 行错误（第五行两个8）
    {
        "行错误（第五行两个8）",
        "5 3 4 6 7 8 9 1 2\n"
        "6 7 2 1 9 5 3 4 8\n"
        "1 9 8 3 4 2 5 6 7\n"
        "8 5 9 7 6 1 4 2 3\n"
        "4 2 6 8 5 3 7 8 1\n"
        "7 1 3 9 2 4 8 5 6\n"
        "9 6 1 5 3 7 2 8 4\n"
        "2 8 7 4 1 9 6 3 5\n"
        "3 4 5 2 8 6 1 7 9\n",
        0, 0, 0, 0
    },

    // 12. 合法数独变体（另一个完全正确的数独）
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
        1, 1, 1, 1
    }
};

// 运行原程序，返回完整输出字符串
char* run_sudoku(const char *input) {
    char temp_in[] = "/tmp/sudoku_in_XXXXXX";
    int fd = mkstemp(temp_in);
    if (fd == -1) return NULL;
    FILE *f = fdopen(fd, "w");
    if (!f) { close(fd); unlink(temp_in); return NULL; }
    fprintf(f, "%s", input);
    fclose(f);

    char command[512];
    snprintf(command, sizeof(command), "./sudoku < %s 2>/dev/null", temp_in);
    FILE *fp = popen(command, "r");
    if (!fp) { unlink(temp_in); return NULL; }

    char buffer[4096] = {0};
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        strncat(buffer, line, sizeof(buffer) - strlen(buffer) - 1);
    }
    pclose(fp);
    unlink(temp_in);
    return strdup(buffer);
}

// 解析输出，获取各项结果
void parse_output(const char *output, int *gong_ok, int *row_ok, int *col_ok, int *final_ok) {
    *gong_ok = 0;
    *row_ok = 0;
    *col_ok = 0;
    *final_ok = 0;

    if (strstr(output, "宫没有问题") != NULL) *gong_ok = 1;
    if (strstr(output, "行没有问题") != NULL) *row_ok = 1;
    if (strstr(output, "列没有问题") != NULL) *col_ok = 1;
    if (strstr(output, "数独没问题") != NULL) *final_ok = 1;
}

int main() {
    if (access("./sudoku", X_OK) != 0) {
        fprintf(stderr, "错误：未找到可执行文件 ./sudoku\n");
        fprintf(stderr, "请先编译原程序：gcc -pthread sudoku.c -o sudoku\n");
        return 1;
    }

    int total = sizeof(test_cases) / sizeof(TestCase);
    int passed = 0;

    printf("===== 数独检查程序完整测试 =====\n");
    printf("测试项：宫、行、列、最终结果\n\n");

    for (int i = 0; i < total; i++) {
        TestCase *tc = &test_cases[i];
        printf("测试[%d]: %s\n", i+1, tc->name);
        char *output = run_sudoku(tc->input);
        if (!output) {
            printf("  ✗ 运行失败\n");
            continue;
        }

        int gong_ok, row_ok, col_ok, final_ok;
        parse_output(output, &gong_ok, &row_ok, &col_ok, &final_ok);

        int fail = 0;
        if (gong_ok != tc->exp_gong_ok) fail = 1;
        if (row_ok != tc->exp_row_ok) fail = 1;
        if (col_ok != tc->exp_col_ok) fail = 1;
        if (final_ok != tc->exp_final_ok) fail = 1;

        if (!fail) {
            printf(" 通过\n");
            passed++;
        } else {
            printf(" 失败\n");
            printf("    期望: 宫=%d, 行=%d, 列=%d, 最终=%d\n",
                   tc->exp_gong_ok, tc->exp_row_ok, tc->exp_col_ok, tc->exp_final_ok);
            printf("    实际: 宫=%d, 行=%d, 列=%d, 最终=%d\n",
                   gong_ok, row_ok, col_ok, final_ok);
            printf("    输出:\n%s\n", output);
        }
        free(output);
    }

    printf("\n总计: %d 个测试, %d 通过, %d 失败\n", total, passed, total - passed);
    return (passed == total) ? 0 : 1;
}
