#include <stdio.h>
#include <stdlib.h>
#define __USE_GNU
#include <sched.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>


#define print_error_then_terminate(en, msg) \
  do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

typedef struct ARG {
    int** X;
    int* y;
    long resSumY;
    long resSumY2;
    long resSumY_2;
    int colSize;
    int rowSize;
    int threadNum;
    int numCores;
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

long sumY2(int* y, int m) {
    long sum = 0;
    for(int i=0; i<m; i++) sum += pow(y[i], 2);
    return sum;
}

long sumX2(int** x, int m, int j) {
    long sum = 0;
    for(int i=0; i<m; i++) sum += pow(x[i][j], 2);
    return sum;
}

long sumXY(int** x, int *y, int m, int j) {
    long sum = 0;
    for(int i=0; i<m; i++) sum += x[i][j] * y[i];
    return sum;
}

long sumXYModified(int** x, int *y, int m, int j, int threadNum) {
    long sum = 0;
    for(int i=0; i<m; i++) sum += x[i][j] * y[i+threadNum*m];
    return sum;
}

long sumX(int** x, int m, int j) {
    long sum = 0;
    for(int i=0; i<m; i++) sum += x[i][j];
    return sum;
}

long sumY(int* y, int m) {
    long sum = 0;
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

void* pearson_cor(void* argsTemp) {
    args_st* args = (args_st*)argsTemp;
    int threadNum = args->threadNum;

    int numCores = args->numCores;
    int coreNum = (threadNum%numCores)+1;
    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);
    CPU_SET(coreNum, &cpu_set);
    int ret = sched_setaffinity(0, sizeof(cpu_set_t), &cpu_set);
    if (ret != 0) {
        perror("sched_setaffinity");
        return NULL;
    }
    printf("Thread%d running on core%d\n", threadNum, coreNum);

    int** X = args->X;
    int* y = args->y;
    long resSumY2 = args->resSumY2;
    long resSumY_2 = args->resSumY_2;
    int m = args->colSize;
    int j = args->rowSize;
    float* v = args->v;
    for(int i=0; i<j; i++) {
        v[i+threadNum*j] = (m*sumXY(X,y,m,i)-sumX(X,m,i)*sumY(y,m))/pow((m*sumX2(X,m,i)-pow(sumX(X,m,i),2))*((m*resSumY2)-resSumY_2),0.5);
    }
    pthread_exit(NULL);
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

void printSubmatrices(int*** subMatrices, int numMatrices, int rowSize, int colSize) {
    printf("\nPRINTING SUBMATRICES\n");
    for(int i=0; i<numMatrices; i++) {
        printf("SUBMATRIX %d\n", i+1);
        for(int j=0; j<colSize; j++) {
            for (int k=0; k<rowSize; k++) printf("%d ", subMatrices[i][j][k]);
            printf("\n");
        }
        printf("\n");
    }
}

void printY(int* y, int size){
    printf("\nPRINTING Y\n");
    for(int i=0; i<size; i++) printf("%d ", y[i]);
    printf("\n");
}

void printResult(float* v, int size) {
    printf("PEARSON CORRELATION COEFFICIENT\n");
    for(int i=0; i<size; i++) printf("%lf ", v[i]);
    printf("\n");
}

int main(int argc, char *argv[]) {
    int size;
    int numOfThreads;
    int num_cores = sysconf(_SC_NPROCESSORS_ONLN)-1;
    char temp;
    bool verbose = false;

    printf("Size: ");
    scanf("%d", &size);
    printf("# of threads: ");
    scanf("%d", &numOfThreads);

    printf("Print matrix? [Y/N]: ");
    scanf(" %c", &temp);
    if (temp == 'Y') { verbose = true; }

    float* v = (float*)malloc(sizeof(float)*size);
    int** matrix = generateRandomMatrix(size);
    int* y = generateRandomY(size);
    if(verbose) printY(y, size);

    pthread_t* tid = (pthread_t*)malloc(sizeof(pthread_t)*numOfThreads);
    
    int*** subMatrices = splitMatrix(matrix, numOfThreads, size);
    if(verbose) printSubmatrices(subMatrices, numOfThreads, size/numOfThreads, size);

    args_st *argsArray = (args_st*)malloc(sizeof(args_st)*numOfThreads);

    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);

    long resSumY = sumY(y,size);
    long resSumY2 = sumY2(y,size);
    long resSumY_2 = pow(resSumY,2);
    for(int i=0; i<numOfThreads; i++) {
        argsArray[i].X = subMatrices[i];
        argsArray[i].y = y;
        argsArray[i].resSumY = resSumY;
        argsArray[i].resSumY2 = resSumY2;
        argsArray[i].resSumY_2 = resSumY_2;
        argsArray[i].colSize = size;
        argsArray[i].rowSize = size/numOfThreads;
        argsArray[i].threadNum = i;
        argsArray[i].numCores = num_cores;
        argsArray[i].v = v;
        pthread_create(&tid[i], NULL, pearson_cor, (void*)&argsArray[i]);
    }

    for(int i=0; i<numOfThreads; i++) pthread_join(tid[i], NULL);
    
    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);
    if(verbose) printResult(v, size);
    printf("time: %f seconds\n", (end.tv_sec-start.tv_sec) + (end.tv_nsec-start.tv_nsec) / 1000000000.0); 
    return 0;
}