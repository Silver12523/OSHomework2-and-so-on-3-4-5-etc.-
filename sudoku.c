#include <stdio.h>
#include <pthread.h>

typedef struct
{
    int row;
    int col;
    int thread_id;
} params;

int sudoku[9][9];
int results_gong[9] = {0}; 
int rows_wrong = 0;
int cols_wrong = 0; //先初始化为0
pthread_t gids[9];
pthread_t rid;
pthread_t cid;

int getting_input() {
    printf("请输入9*9数独\n");
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            scanf("%d", &sudoku[i][j]);
        }
    }
    return 0;
}

void* check_gong(void* parameters) { 
    params* p = (params*) parameters;
    int used_gong[10] = {0};
    for (int i = p->row; i < (p->row + 3); i++) {
        for (int j = p->col; j < (p->col + 3); j++) {
            int target = sudoku[i][j];
            if (target < 1 || target > 9 || used_gong[target]) {
                results_gong[p->thread_id] = 1; //如果不符合要求返回1
                pthread_exit(NULL); //退出线程而不是单纯return
            }
            used_gong[target] = 1;
        }
    }
    return NULL;
}
void* check_row(void* arg) {
    for (int i = 0; i < 9; i++) {
        int used_row[10] = {0};
        for (int j = 0; j < 9; j++) {
            int target = sudoku[i][j];
            if (target < 1 || target > 9 || used_row[target]) {
                rows_wrong = 1;
                pthread_exit(NULL);
            }
            used_row[target] = 1;
        }
    }
    return NULL;
}
void* check_col(void* arg) {
    for (int i = 0; i < 9; i++) {
        int used_col[10] = {0};
        for (int j = 0; j < 9; j++) {
            int target = sudoku[j][i];
            if (target < 1 || target > 9 || used_col[target]) {
                cols_wrong = 1;
                pthread_exit(NULL);
            }
            used_col[target] = 1;
        }
    }
    return NULL;
}

int main() {
    getting_input();
    params input[9];
    for (int i = 0; i < 9; i++) {
        input[i].row = (i / 3) * 3;
        input[i].col = (i % 3) * 3;
        input[i].thread_id = i;
        pthread_create(&gids[i], NULL, check_gong, &input[i]); //what are the parameters?
    }
    pthread_create(&rid, NULL, check_row, NULL);
    pthread_create(&cid, NULL, check_col, NULL);

    for (int k = 0; k < 9; k++) {
        pthread_join(gids[k], NULL);
    }
    pthread_join(rid, NULL);
    pthread_join(cid, NULL);
    //等待所有子线程算完
    int is_valid = 1;
    for (int j = 0; j < 9; j++) {
        if(results_gong[j] == 1) {
            printf("不符合要求，第%d宫有问题。\n", j + 1);
            is_valid = 0;
        }
    }
    if (is_valid) printf("宫没有问题。\n");
    if (rows_wrong) {
        printf("不符合要求，行有问题。\n");
        is_valid = 0;
    } else printf("行没有问题。\n");

    if (cols_wrong) {
        printf("不符合要求，列有问题。\n");
        is_valid = 0;
    } else printf("列没有问题。\n");
    if (is_valid) {
        printf("数独没问题。\n");
        return 0;
    } else return -1;
}