#include <stdio.h>
#include <stdlib.h>
#define __USE_GNU
#include <sched.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

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

void pearson_cor(int** X, int* y, int m, int n, float* v) {

    long resSumY = sumY(y,m);
    long resSumY2 = sumY2(y,m);
    long resSumY_2 = pow(resSumY,2);

    for(int i=0; i<m; i++) {
        v[i] = (m*sumXY(X,y,m,i)-sumX(X,m,i)*resSumY)/pow((m*sumX2(X,m,i)-pow(sumX(X,m,i),2))*((m*resSumY2)-resSumY_2),0.5);
    }
}

void printMatrix(int **X, int size) {
    for(int i=0; i<size; i++) {
        for (int j=0; j<size; j++) {
            printf("%d ", X[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    int size;

    printf("Size: ");
    scanf("%d", &size);

    float* v = (float*)malloc(sizeof(float)*size);
    int** matrix = generateRandomMatrix(size);
    // printMatrix(matrix, size);
    int* y = generateRandomY(size);
    // for(int i=0; i<size; i++) printf("%d ", y[i]);
    printf("\n");

    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);

    pearson_cor(matrix, y, size, size, v);
    
    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("time: %f seconds\n", (end.tv_sec-start.tv_sec) + (end.tv_nsec-start.tv_nsec) / 1000000000.0); 
    // for(int i=0; i<size; i++) printf("%lf ", v[i]);
    printf("\n");
    return 0;
}