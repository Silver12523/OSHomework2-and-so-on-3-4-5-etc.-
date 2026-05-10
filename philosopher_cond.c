#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define N 5  
#define LEFT (i + N - 1) % N  // 左边哲学家
#define RIGHT (i + 1) % N     // 右边哲学家

// 哲学家状态定义
#define THINKING 0
#define HUNGRY   1
#define EATING   2

int state[N];                  // 状态数组
pthread_mutex_t mutex;         // 全局互斥锁
pthread_cond_t cond[N];       // 条件变量数组
pthread_t tid[N];             // 哲学家线程ID
int philo_id[N];               // 哲学家编号


// 吃饭：自己饥饿&左右都没吃饭
void test(int i) {
    if (state[i] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING) {
        state[i] = EATING;
        printf("哲学家 %d 开始吃饭\n", i);
        pthread_cond_signal(&cond[i]);
    }
}

// 饥饿：等筷子拿筷子
void take_forks(int i) {
    pthread_mutex_lock(&mutex);
    state[i] = HUNGRY;
    printf("哲学家 %d 饥饿，等待筷子...\n", i);
    test(i);
    while (state[i] != EATING) {
        pthread_cond_wait(&cond[i], &mutex);
    }
    pthread_mutex_unlock(&mutex);
}

// 吃完了放筷子
void put_forks(int i) {
    pthread_mutex_lock(&mutex);
    state[i] = THINKING;
    printf("哲学家 %d 吃完，放下筷子，开始思考\n", i);
    test(LEFT);
    test(RIGHT);
    pthread_mutex_unlock(&mutex);
}

// 思考
void think(int i) {
    printf("哲学家 %d 正在思考...\n", i);
    sleep(rand() % 2 + 1);  
}

// 吃饭
void eat(int i) {
    sleep(rand() % 2 + 1);  
}

void* philosopher(void* num) {
    int i = *(int*)num;
    while (1) {
        think(i);      
        take_forks(i); 
        eat(i);        
        put_forks(i);  
    }
}

int main() {
    srand(time(NULL));  

    // 初始化
    pthread_mutex_init(&mutex, NULL);
    for (int i = 0; i < N; i++) {
        pthread_cond_init(&cond[i], NULL);
        state[i] = THINKING;
        philo_id[i] = i;
    }

    printf("哲学家就餐问题模拟：\n\n");
    for (int i = 0; i < N; i++) {
        pthread_create(&tid[i], NULL, philosopher, &philo_id[i]);
    }

    // 等待线程结束&销毁资源
    for (int i = 0; i < N; i++) {
        pthread_join(tid[i], NULL);
    }
    pthread_mutex_destroy(&mutex);
    for (int i = 0; i < N; i++) {
        pthread_cond_destroy(&cond[i]);
    }

    return 0;
}
