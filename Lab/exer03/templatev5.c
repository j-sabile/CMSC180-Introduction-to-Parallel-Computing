#include <stdio.h>
#include <stdlib.h>
#define __USE_GNU
#include <sched.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

typedef struct ARGS {
    int threadNum;
} args_st;

void *thread_func(void *argsTemp) {
    args_st* args = (args_st*)argsTemp;
    int threadNum = args->threadNum;

    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);

    CPU_SET(threadNum, &cpu_set);

    int ret = sched_setaffinity(0, sizeof(cpu_set_t), &cpu_set);
    if (ret != 0) {
        perror("sched_setaffinity");
        return NULL;
    }

    printf("Thread running on core %d\n", threadNum);

    long cnt = 0;
    for(int i=0; i<pow(10,10); i++) {
        cnt++;
    }
    printf("%ld\n", cnt);

    return NULL;
}

int main() {
    int numThreads;
    struct timespec start, end;

    printf("# threads: ");
    scanf("%d", &numThreads);

    args_st* argsArray = (args_st*)malloc(sizeof(args_st)*numThreads);
    pthread_t* tid = (pthread_t*)malloc(sizeof(pthread_t)*numThreads);

    clock_gettime(CLOCK_MONOTONIC, &start);
    
    for(int i=0; i<numThreads; i++) {
        argsArray[i].threadNum = i;
        pthread_create(&tid[i], NULL, thread_func, (void*)&argsArray[i]);
    }

    for(int i=0; i<numThreads; i++) pthread_join(tid[i], NULL);

    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("time: %f seconds\n", (end.tv_sec-start.tv_sec) + (end.tv_nsec-start.tv_nsec) / 1000000000.0); 

    return 0;
}