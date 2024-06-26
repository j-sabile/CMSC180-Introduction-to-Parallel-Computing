#include <stdio.h>
#include <stdlib.h>
#define __USE_GNU
#include <sched.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#define SIZE 1000000000

typedef struct ARG {
    int threadNum;
    int numCores;
    int* y;
} args_st;
 
// The <errno.h> header file defines the integer variable errno, which is set by system calls and some library functions in the event of an error to indicate what went wrong.
#define print_error_then_terminate(en, msg) \
  do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)
 
long test(int* y) {
    long cnt = 0;
    for(int i=0; i<SIZE; i++) {
        cnt+=y[i];
    }
    return cnt;
}

void* threadStart(void* argsTemp) {
    args_st* args = (args_st*)argsTemp;
    
    int threadNum = args->threadNum;
    int numCores = args->numCores;
    int* y = args->y;

    int core_id = threadNum%numCores; // Assign to the second core
    const pid_t pid = getpid();
    
    // cpu_set_t: This data set is a bitset where each bit represents a CPU.
    cpu_set_t cpuset;
    // CPU_ZERO: This macro initializes the CPU set set to be the empty set.
    CPU_ZERO(&cpuset);
    // CPU_SET: This macro adds cpu to the CPU set set.
    CPU_SET(core_id, &cpuset);
    
    // sched_setaffinity: This function installs the cpusetsize bytes long affinity mask pointed to by cpuset for the process or thread with the ID pid. If successful the function returns zero and the scheduler will in future take the affinity information into account.
    const int set_result = sched_setaffinity(pid, sizeof(cpu_set_t), &cpuset);
    if (set_result != 0) print_error_then_terminate(set_result, "sched_setaffinity");
    // Check what is the actual affinity mask that was assigned to the thread.
    // sched_getaffinity: This functions stores the CPU affinity mask for the process or thread with the ID pid in the cpusetsize bytes long bitmap pointed to by cpuset. If successful, the function always initializes all bits in the cpu_set_t object and returns zero.
    const int get_affinity = sched_getaffinity(pid, sizeof(cpu_set_t), &cpuset);
    if (get_affinity != 0) print_error_then_terminate(get_affinity, "sched_getaffinity");
    // CPU_ISSET: This macro returns a nonzero value (true) if cpu is a member of the CPU set set, and zero (false) otherwise.
    if (CPU_ISSET(core_id, &cpuset)) fprintf(stdout, "Successfully set thread %d to affinity to CPU %d\n", pid, core_id);
    else fprintf(stderr, "Failed to set thread %d to affinity to CPU %d\n", pid, core_id);

    printf("%ld\n",test(y));
    pthread_exit(NULL);
}



int* generateRandomY(int size){
    srand(time(NULL)); 
    int* temp = (int*)malloc(sizeof(int)*size);
    for(int i=0; i<size; i++) temp[i] = rand()%6+1;
    return temp;
}

int main(int argc, char *argv[]) {
  int numOfThreads = 8;
  pthread_t* tid = (pthread_t*)malloc(sizeof(pthread_t)*numOfThreads);
  args_st *argsArray = (args_st*)malloc(sizeof(args_st)*numOfThreads);
//   int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
  int num_cores = 1;

  int* y = generateRandomY(SIZE);


  struct timespec start;
  clock_gettime(CLOCK_MONOTONIC, &start);

  for(int i=0; i<numOfThreads; i++) {
    argsArray[i].threadNum = i;
    argsArray[i].numCores = num_cores;
    argsArray[i].y = y;
    pthread_create(&tid[i], NULL, threadStart, (void*)&argsArray[i]);
  }

  for(int i=0; i<numOfThreads; i++) {
    pthread_join(tid[i], NULL);
  }

  struct timespec end;
  clock_gettime(CLOCK_MONOTONIC, &end);
  printf("time: %f seconds\n", (end.tv_sec-start.tv_sec) + (end.tv_nsec-start.tv_nsec) / 1000000000.0); 
        
 
  return 0;
}