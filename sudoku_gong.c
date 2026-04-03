#include <stdio.h>
#include <pthread.h>

typedef struct
{
    int row;
    int col;
    int thread_id;
} params;

int sudoku[9][9];
int results[9] = {0}; //先初始化为0
pthread_t tids[9];

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
    int used[9] = {0};
    for (int i = p->row; i < (p->row + 3); i++) {
        for (int j = p->col; j < (p->col + 3); j++) {
            int target = sudoku[i][j];
            if (target < 1 || target > 9 || used[target - 1]) {
                results[p->thread_id] = 1; //如果不符合要求返回1
                pthread_exit(NULL); //退出线程而不是单纯return
            }
            used[target - 1] = 1;
        }
    }
    return NULL;
}

int main() {
    params input[9];
    for (int i = 0; i < 9; i++) {
        input[i].row = (i / 3) * 3;
        input[i].col = (i % 3) * 3;
        input[i].thread_id = i;
        pthread_create(&tids[i], NULL, check_gong, &input[i]); //what are the parameters?
    }
    for (int k = 0; k < 9; k++) {
        pthread_join(tids[k], NULL);
    }
    //等待所有子线程算完
    for (int j = 0; j < 9; j++) {
        if(results[j] == 1) {
            printf("不符合要求，宫有问题。\n");
            return -1;
        }
    }
    printf("宫没有问题。\n");
    return 0;
}