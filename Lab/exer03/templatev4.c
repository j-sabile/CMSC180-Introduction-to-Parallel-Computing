#include <stdio.h>
#include <stdlib.h>
#define __USE_GNU
#include <sched.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

// #define CORE0 (1 << 0)  // Binary representation of core 0
// #define CORE1 (1 << 1)  // Binary representation of core 1

void *thread_func(void *arg) {


    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);
    const pid_t pid = getpid();

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

    printf("Thread running on core %ld %d\n", (long)arg, pid);

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
    pthread_t thread0, thread1;

    pthread_create(&thread0, NULL, thread_func, (void *)0);
    pthread_create(&thread1, NULL, thread_func, (void *)1);

    pthread_join(thread0, NULL);
    pthread_join(thread1, NULL);

    return 0;
}