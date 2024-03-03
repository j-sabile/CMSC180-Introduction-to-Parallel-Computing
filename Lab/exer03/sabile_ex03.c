#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<math.h>
#include<time.h>
#define __USE_GNU
#include <sched.h>
#include <errno.h>
#include <unistd.h>

typedef struct ARG {
    int** X;
    int* y;
    long resSumY;
    long resSumY2;
    long resSumY_2;
    int colSize;
    int rowSize;
    int threadNum;
    float* v;
} args_st;

typedef struct ARG2 {
    int** X;
    int* y;
    int colSize;
    int rowSize;
    int threadNum;
    long* resSumX;
    long* resSumX2;
    long* resSumXY;
} args_st2;

int sumY2(int* y, int m) {
    int sum = 0;
    for(int i=0; i<m; i++) sum += pow(y[i], 2);
    return sum;
}

long sumX2(int** x, int m, int j) {
    int sum = 0;
    for(int i=0; i<m; i++) sum += pow(x[i][j], 2);
    return sum;
}

int sumXY(int** x, int *y, int m, int j) {
    int sum = 0;
    for(int i=0; i<m; i++) sum += x[i][j] * y[i];
    return sum;
}

int sumXYModified(int** x, int *y, int m, int j, int threadNum) {
    int sum = 0;
    for(int i=0; i<m; i++) sum += x[i][j] * y[i+threadNum*m];
    return sum;
}

int sumX(int** x, int m, int j) {
    int sum = 0;
    for(int i=0; i<m; i++) sum += x[i][j];
    return sum;
}

int sumY(int* y, int m) {
    int sum = 0;
    for(int i=0; i<m; i++) sum += y[i];
    return sum;
}

int** generateRandomMatrix(int size) {
    srand(time(NULL)); 
    int** temp = (int**)malloc(sizeof(int*)*size);
    for(int i=0; i<size; i++) {
        temp[i] = (int*)malloc(sizeof(int)*size);
        for (int j=0; j<size; j++) temp[i][j] = rand()%100+1;
    }
    return temp;
}

int* generateRandomY(int size){
    srand(time(NULL)); 
    int* temp = (int*)malloc(sizeof(int)*size);
    for(int i=0; i<size; i++) temp[i] = rand()%100+1;
    return temp;
}

// The <errno.h> header file defines the integer variable errno, which is set by system calls and some library functions in the event of an error to indicate what went wrong.
#define print_error_then_terminate(en, msg) \
  do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

int num_cores; // Number of cores available 
int next_core = 0; // Counter for round-robin assignment
void* pearson_cor(void* argsTemp) {



    args_st* args = (args_st*)argsTemp;
    int** X = args->X;
    int* y = args->y;
    long resSumY = args->resSumY;
    long resSumY2 = args->resSumY2;
    long resSumY_2 = args->resSumY_2;
    int m = args->colSize;
    int j = args->rowSize;
    int threadNum = args->threadNum;
    float* v = args->v;
    for(int i=0; i<j; i++) {
        printf("%d %ld %f\n", m, sumX2(X,m,i), pow(sumX(X,m,i),2));
        v[i+threadNum*j] = (m*sumXY(X,y,m,i)-sumX(X,m,i)*sumY(y,m))/pow((m*sumX2(X,m,i)-pow(sumX(X,m,i),2))*((m*resSumY2)-resSumY_2),0.5);
    }
    pthread_exit(NULL);
    return NULL;
}

int*** splitMatrix(int** matrix, int numOfThreads, int size) {
    int*** temp = (int***)malloc(sizeof(int**)*numOfThreads);
    int rowSize = size/numOfThreads;
    for(int i=0; i<numOfThreads; i++) {
        temp[i] = (int**)malloc(sizeof(int*)*size);
        for(int j=0; j<size; j++) {
            temp[i][j] = (int*)malloc(sizeof(int)*size);
            for(int k=0; k<rowSize; k++) temp[i][j][k] = matrix[j][k+rowSize*i];
        }
    }
    return temp;
}

int*** splitMatrixByRow(int** matrix, int numOfThreads, int size) {
    int*** temp = (int***)malloc(sizeof(int**)*numOfThreads);
    int colSize = size/numOfThreads;
    for(int i=0; i<numOfThreads; i++) {
        temp[i] = (int**)malloc(sizeof(int*)*colSize);
        for(int j=0; j<colSize; j++) {
            temp[i][j] = (int*)malloc(sizeof(int)*size);
            for(int k=0; k<size; k++) temp[i][j][k] = matrix[j+colSize*i][k];
        }
    }
    return temp;
}

void printSubmatrices(int*** subMatrices, int size, int numOfThreads) {
    int colSize = size / numOfThreads;
    printf("\nPRINTING SUBMATRICES\n");
    for(int i=0; i<numOfThreads; i++) {
        printf("SUBMATRIX %d\n", i+1);
        for(int j=0; j<colSize; j++) {
            for (int k=0; k<size; k++) printf("%d ", subMatrices[i][j][k]);
            printf("\n");
        }
        printf("\n");
    }
}

void* getSumOfSubCol(void* argsTemp) {
    args_st2* args = (args_st2*)argsTemp;
    int** X = args->X;
    int* y = args->y;
    int m = args->colSize;
    int j = args->rowSize;
    int threadNum = args->threadNum;
    for(int i=0; i<j; i++) {
        args->resSumX[i] = sumX(X,m,i);
        args->resSumX2[i] = sumX2(X,m,i);
        args->resSumXY[i] = sumXYModified(X,y,m,i,threadNum);
    }
    pthread_exit(NULL);
    return NULL;
}

int main(int argc, char *argv[]) {
    num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    int core_id = next_core; // Assign to the second core
    next_core = (next_core + 1) % num_cores;  // Update for the next thread
    const pid_t pid = getpid();

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);

    if (sched_setaffinity(pthread_self(), sizeof(cpuset), &cpuset) != 0) {
        perror("sched_setaffinity");
    }

    // sched_setaffinity: This function installs the cpusetsize bytes long affinity mask pointed to by cpuset for the process or thread with the ID pid. If successful the function returns zero and the scheduler will in future take the affinity information into account.
    const int set_result = sched_setaffinity(pid, sizeof(cpu_set_t), &cpuset);
    if (set_result != 0) {
    
        print_error_then_terminate(set_result, "sched_setaffinity");
    }
    
    // Check what is the actual affinity mask that was assigned to the thread.
    // sched_getaffinity: This functions stores the CPU affinity mask for the process or thread with the ID pid in the cpusetsize bytes long bitmap pointed to by cpuset. If successful, the function always initializes all bits in the cpu_set_t object and returns zero.
    const int get_affinity = sched_getaffinity(pid, sizeof(cpu_set_t), &cpuset);
    if (get_affinity != 0) {
    
        print_error_then_terminate(get_affinity, "sched_getaffinity");
    }
    
    // CPU_ISSET: This macro returns a nonzero value (true) if cpu is a member of the CPU set set, and zero (false) otherwise.
    if (CPU_ISSET(core_id, &cpuset)) {
    
        fprintf(stdout, "Successfully set thread %d to affinity to CPU %d\n", pid, core_id);
    } else {
    
        fprintf(stderr, "Failed to set thread %d to affinity to CPU %d\n", pid, core_id);
    }



    char input;
    int size;
    int numOfThreads;

    printf("Split by Column or Row (C/R): ");
    scanf(" %c", &input);

    printf("Size: ");
    scanf("%d", &size);
    printf("# of threads: ");
    scanf("%d", &numOfThreads);

    float* v = (float*)malloc(sizeof(float)*size);

    int** matrix = generateRandomMatrix(size);
    int* y = generateRandomY(size);
    pthread_t* tid = (pthread_t*)malloc(sizeof(pthread_t)*numOfThreads);
    
    int*** subMatrices = splitMatrixByRow(matrix, numOfThreads, size);
    // printSubmatrices(subMatrices, size, numOfThreads);

    if (input == 'C') {
        args_st *argsArray = (args_st*)malloc(sizeof(args_st)*numOfThreads);

        struct timespec start;
        clock_gettime(CLOCK_MONOTONIC, &start);

        long resSumY = sumY(y,size);
        long resSumY2 = sumY2(y,size);
        long resSumY_2 = pow(resSumY,2);
        printf("%ld %ld %ld\n", resSumY, resSumY2, resSumY_2);
        for(int i=0; i<numOfThreads; i++) {
            argsArray[i].X = subMatrices[i];
            argsArray[i].y = y;
            argsArray[i].resSumY = resSumY;
            argsArray[i].resSumY2 = resSumY2;
            argsArray[i].resSumY_2 = resSumY_2;
            argsArray[i].colSize = size;
            argsArray[i].rowSize = size/numOfThreads;
            argsArray[i].threadNum = i;
            argsArray[i].v = v;
            pthread_create(&tid[i], NULL, pearson_cor, (void*)&argsArray[i]);
        }

        for(int i=0; i<numOfThreads; i++) pthread_join(tid[i], NULL);
        
        struct timespec end;
        clock_gettime(CLOCK_MONOTONIC, &end);
        printf("time: %f seconds\n", (end.tv_sec-start.tv_sec) + (end.tv_nsec-start.tv_nsec) / 1000000000.0); 
        for(int i=0; i<size; i++) printf("%lf ", v[i]);
        printf("\n");
        return 0;
    } else if (input == 'R') {
        args_st2 *argsArray = (args_st2*)malloc(sizeof(args_st2)*numOfThreads);

        struct timespec start;
        clock_gettime(CLOCK_MONOTONIC, &start);

        for(int i=0; i<numOfThreads; i++) {
            argsArray[i].X = subMatrices[i];
            argsArray[i].y = y;
            argsArray[i].colSize = size/numOfThreads;
            argsArray[i].rowSize = size;
            argsArray[i].threadNum = i;
            argsArray[i].resSumX = (long*)malloc(sizeof(long)*size);
            argsArray[i].resSumX2 = (long*)malloc(sizeof(long)*size);
            argsArray[i].resSumXY = (long*)malloc(sizeof(long)*size);
            pthread_create(&tid[i], NULL, getSumOfSubCol, (void*)&argsArray[i]);
        }

        for(int i=0; i<numOfThreads; i++) pthread_join(tid[i], NULL);
        
        long resSumY = sumY(y,size);
        long resSumY2 = sumY2(y,size);
        long resSumY_2 = pow(resSumY,2);

        for(int i=0; i<size; i++){
            long resSumX = 0;
            long resSumX2 = 0;
            long resSumXY = 0;
            for(int j=0; j<numOfThreads; j++) resSumX += argsArray[j].resSumX[i];
            for(int j=0; j<numOfThreads; j++) resSumX2 += argsArray[j].resSumX2[i];
            for(int j=0; j<numOfThreads; j++) resSumXY += argsArray[j].resSumXY[i];
            v[i] = (size*resSumXY-resSumX*resSumY)/pow((size*resSumX2-pow(resSumX,2))*(size*resSumY2-resSumY_2),0.5);
        }

        struct timespec end;
        clock_gettime(CLOCK_MONOTONIC, &end);
        printf("time: %f seconds\n", (end.tv_sec-start.tv_sec) + (end.tv_nsec-start.tv_nsec) / 1000000000.0); 
        for(int i=0; i<size; i++) printf("%lf ", v[i]);
        return 0;
    }
}