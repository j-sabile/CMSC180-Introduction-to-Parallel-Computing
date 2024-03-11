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

// #define CORE0 (1 << 0)  // Binary representation of core 0
// #define CORE1 (1 << 1)  // Binary representation of core 1

void *thread_func(void *arg) {


    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);

    if ((long)arg == 0) {
        CPU_SET(0, &cpu_set);  // Set CPU affinity for core0
    } else {
        CPU_SET(1, &cpu_set);  // Set CPU affinity for core1
    }

    int ret = sched_setaffinity(0, sizeof(cpu_set_t), &cpu_set);
    if (ret != 0) {
        perror("sched_setaffinity");
        return NULL;
    }

    printf("Thread running on core %ld\n", (long)arg);

    // Simulating some work
    // sleep(5);
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
    scanf("%d ", &numThreads);

    args_st* argsArray = (args_st*)malloc(sizeof(args_st)*numThreads);

    clock_gettime(CLOCK_MONOTONIC, &start);
    
    for(int i=0; i<numThreads; i++) {
        argsArray[i].threadNum = i;
        pthread_create()
    }
    pthread_create(&thread0, NULL, thread_func, (void *)0);
    pthread_create(&thread1, NULL, thread_func, (void *)1);

    pthread_join(thread0, NULL);
    pthread_join(thread1, NULL);

    return 0;
}