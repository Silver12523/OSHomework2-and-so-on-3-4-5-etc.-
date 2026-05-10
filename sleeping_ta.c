#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define HALLWAY_CHAIRS 3
#define DEFAULT_STUDENTS 5
#define HELP_ROUNDS 3

typedef struct {
    int id;
    unsigned int seed;
} student_arg_t;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static sem_t wakeupTA;
static sem_t *student_called;
static sem_t *help_done;

static int *waiting_queue;
static int queue_front = 0;
static int queue_rear = 0;
static int waiting_count = 0;

static int student_count = 0;
static int finished_students = 0;
static int ta_sleeping = 1;

static void enqueue_student(int student_id) {
    waiting_queue[queue_rear] = student_id;
    queue_rear = (queue_rear + 1) % HALLWAY_CHAIRS;
    waiting_count++;
} //学生排队

static int dequeue_student(void) {
    int student_id = waiting_queue[queue_front];
    queue_front = (queue_front + 1) % HALLWAY_CHAIRS;
    waiting_count--;
    return student_id;
} //学生出队

static int random_between(unsigned int *seed, int min_ms, int max_ms) {
    int span = max_ms - min_ms + 1;
    return min_ms + (int)(rand_r(seed) % span);
} //随机时间

static void sleep_ms(int milliseconds) {
    usleep((useconds_t)milliseconds * 1000U);
} //睡觉时间

static void *ta_thread(void *arg) {
    unsigned int seed = (unsigned int)time(NULL) ^ 0xABCDEFU; //生成随机数
    (void)arg;

    while (1) {
        pthread_mutex_lock(&mutex);
        if (waiting_count == 0 && finished_students < student_count) {
            ta_sleeping = 1; //现在没人在等而且还有学生没有完成
            printf("TA: 睡觉。\n");
        }
        pthread_mutex_unlock(&mutex);//更改信号量的时候必须锁mutex
        sem_wait(&wakeupTA); //学生唤醒TA的阻塞点
        pthread_mutex_lock(&mutex);
        ta_sleeping = 0;
        if (waiting_count == 0 && finished_students == student_count) {
            pthread_mutex_unlock(&mutex);
            printf("TA: 答疑结束。\n");
            break;
        }
        if (waiting_count == 0) {
            pthread_mutex_unlock(&mutex);
            continue;
        }
        int student_id = dequeue_student();
        printf("TA: 请 %d 号学生进办公室，当前走廊还剩 %d 人等待。\n",
               student_id + 1, waiting_count);
        pthread_mutex_unlock(&mutex);
        sem_post(&student_called[student_id]);
        sleep_ms(random_between(&seed, 500, 1200));//用睡眠模拟辅导时间
        printf("TA: 已完成对 %d 号学生的辅导。\n", student_id + 1);
        sem_post(&help_done[student_id]);
    }
    return NULL;
}
static void *student_thread(void *arg) {
    student_arg_t *student = (student_arg_t *)arg;
    for (int round = 1; round <= HELP_ROUNDS; round++) {
        sleep_ms(random_between(&student->seed, 300, 1400));
        printf("学生 %d: 编程时遇到问题，准备去找 TA。(%d/%d)\n",
               student->id + 1, round, HELP_ROUNDS);
        pthread_mutex_lock(&mutex);
        if (waiting_count < HALLWAY_CHAIRS) {
            if (ta_sleeping) {
                printf("学生 %d: 发现 TA 在睡觉，先把 TA 叫醒。\n",
                       student->id + 1);
            }
            enqueue_student(student->id);
            printf("学生 %d: 坐到走廊椅子上等待，当前等待人数 %d。\n",
                   student->id + 1, waiting_count);
            pthread_mutex_unlock(&mutex);
            sem_post(&wakeupTA);//叫醒TA
            sem_wait(&student_called[student->id]);//等待TA唤醒
            printf("学生 %d: 进入办公室接受辅导。\n", student->id + 1);
            sem_wait(&help_done[student->id]);//等待TA完成辅导
            printf("学生 %d: 本轮问题已解决，回去继续编程。\n",
                   student->id + 1);
        } else {
            printf("学生 %d: 走廊没有空椅子，先回去继续编程，稍后再来。\n",
                   student->id + 1);
            pthread_mutex_unlock(&mutex);
            round--;
        }
    }
    pthread_mutex_lock(&mutex);
    finished_students++;
    printf("学生 %d: 本次所有求助都完成了，结束线程。(%d/%d)\n",
           student->id + 1, finished_students, student_count);
    int all_finished = (finished_students == student_count);
    pthread_mutex_unlock(&mutex);
    if (all_finished) {
        sem_post(&wakeupTA);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t ta;
    pthread_t *students;
    student_arg_t *student_args;
    int configured_students;
    student_count = DEFAULT_STUDENTS;
    if (argc > 1) {
        char *endptr = NULL;
        long parsed = strtol(argv[1], &endptr, 10);
        if (endptr == argv[1] || *endptr != '\0' || parsed <= 0) {
            fprintf(stderr, "用法: %s [学生数量(正整数)]\n", argv[0]);
            return EXIT_FAILURE;
        }
        student_count = (int)parsed;
    }//命令行自定义参数
    configured_students = student_count;
    waiting_queue = (int *)malloc(sizeof(int) * HALLWAY_CHAIRS);
    students = (pthread_t *)malloc(sizeof(pthread_t) * (size_t)student_count);
    student_args = (student_arg_t *)malloc(sizeof(student_arg_t) * (size_t)student_count);
    student_called = (sem_t *)malloc(sizeof(sem_t) * (size_t)student_count);
    help_done = (sem_t *)malloc(sizeof(sem_t) * (size_t)student_count);
    if (waiting_queue == NULL || students == NULL || student_args == NULL ||
        student_called == NULL || help_done == NULL) {
        fprintf(stderr, "内存分配失败。\n");
        free(waiting_queue);
        free(students);
        free(student_args);
        free(student_called);
        free(help_done);
        return EXIT_FAILURE;
    }//内存分配和释放
    if (sem_init(&wakeupTA, 0, 0) != 0) {
        perror("sem_init");
        free(waiting_queue);
        free(students);
        free(student_args);
        free(student_called);
        free(help_done);
        return EXIT_FAILURE;
    }
    for (int i = 0; i < student_count; i++) {
        if (sem_init(&student_called[i], 0, 0) != 0) {
            perror("sem_init");
            for (int j = 0; j < i; j++) {
                sem_destroy(&student_called[j]);
                sem_destroy(&help_done[j]);
            }
            sem_destroy(&wakeupTA);
            free(waiting_queue);
            free(students);
            free(student_args);
            free(student_called);
            free(help_done);
            return EXIT_FAILURE;
        }

        if (sem_init(&help_done[i], 0, 0) != 0) {
            perror("sem_init");
            sem_destroy(&student_called[i]);
            for (int j = 0; j < i; j++) {
                sem_destroy(&student_called[j]);
                sem_destroy(&help_done[j]);
            }
            sem_destroy(&wakeupTA);
            free(waiting_queue);
            free(students);
            free(student_args);
            free(student_called);
            free(help_done);
            return EXIT_FAILURE;
        }
    }

    if (pthread_create(&ta, NULL, ta_thread, NULL) != 0) {
        perror("pthread_create");
        sem_destroy(&wakeupTA);
        for (int i = 0; i < student_count; i++) {
            sem_destroy(&student_called[i]);
            sem_destroy(&help_done[i]);
        }
        free(waiting_queue);
        free(students);
        free(student_args);
        free(student_called);
        free(help_done);
        return EXIT_FAILURE;
    }

    for (int i = 0; i < student_count; i++) {
        student_args[i].id = i;
        student_args[i].seed = (unsigned int)time(NULL) ^ (unsigned int)(i * 7919 + 12345);
        if (pthread_create(&students[i], NULL, student_thread, &student_args[i]) != 0) {
            perror("pthread_create");
            pthread_mutex_lock(&mutex);
            student_count = i;
            pthread_mutex_unlock(&mutex);
            sem_post(&wakeupTA);
            pthread_join(ta, NULL);
            for (int j = 0; j < i; j++) {
                pthread_join(students[j], NULL);
            }
            sem_destroy(&wakeupTA);
            for (int j = 0; j < configured_students; j++) {
                sem_destroy(&student_called[j]);
                sem_destroy(&help_done[j]);
            }
            free(waiting_queue);
            free(students);
            free(student_args);
            free(student_called);
            free(help_done);
            return EXIT_FAILURE;
        }
    }
    for (int i = 0; i < student_count; i++) {
        pthread_join(students[i], NULL);
    }
    pthread_join(ta, NULL);
    sem_destroy(&wakeupTA);
    for (int i = 0; i < student_count; i++) {
        sem_destroy(&student_called[i]);
        sem_destroy(&help_done[i]);
    }
    pthread_mutex_destroy(&mutex);
    free(waiting_queue);
    free(students);
    free(student_args);
    free(student_called);
    free(help_done);
    return EXIT_SUCCESS;
}
