#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<math.h>
#include<time.h>

typedef struct ARG {
    int** X;
    int* y;
    int colSize;
    int rowSize;
    int threadNum;
    float* v;
} args_st;

int sumY2(int* y, int m) {
    int sum = 0;
    for(int i=0; i<m; i++) sum += pow(y[i], 2);
    return sum;
}

int sumX2(int** x, int m, int j) {
    int sum = 0;
    for(int i=0; i<m; i++) sum += pow(x[i][j], 2);
    return sum;
}

int sumXY(int** x, int *y, int m, int j) {
    int sum = 0;
    for(int i=0; i<m; i++) sum += x[i][j] * y[i];
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

void* pearson_cor(void* argsTemp) {
    args_st* args = (args_st*)argsTemp;
    int** X = args->X;
    int* y = args->y;
    int m = args->colSize;
    int j = args->rowSize;
    int threadNum = args->threadNum;
    float* v = args->v;
    for(int i=0; i<j; i++) {
        v[i+threadNum*j] = (m*sumXY(X,y,m,i)-sumX(X,m,i)*sumY(y,m))/pow((m*sumX2(X,m,i)-pow(sumX(X,m,i),2))*((m*sumY2(y,m))-pow(sumY(y,m),2)),0.5);
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

int main() {
    int size;
    int numOfThreads;

    printf("Size: ");
    scanf("%d", &size);
    printf("# of threads: ");
    scanf("%d", &numOfThreads);

    float* v = (float*)malloc(sizeof(float)*size);

    int** matrix = generateRandomMatrix(size);
    int*** subMatrices = splitMatrix(matrix, numOfThreads, size);
    int* y = generateRandomY(size);
    
    pthread_t* tid = (pthread_t*)malloc(sizeof(pthread_t)*numOfThreads);
    args_st *argsArray = (args_st*)malloc(sizeof(args_st)*numOfThreads);

    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for(int i=0; i<numOfThreads; i++) {
        argsArray[i].X = subMatrices[i];
        argsArray[i].y = y;
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
    // for(int i=0; i<size; i++) printf("%lf ", v[i]);
    return 0;
}
