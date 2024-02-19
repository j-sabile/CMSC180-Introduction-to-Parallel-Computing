#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<math.h>
#include<time.h>

// args for the test thread
// typedef struct ARG {
//     int* y;
//     int size;
// } args_st;

typedef struct ARG {
    int** X;
    int* y;
    int colSize;
    int rowSize;
    int threadNum;
    int* v;
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
    int* v = args->v;
    for(int i=0; i<j; i++) {
        printf("i=%d threadNum=%i j=%d\n", i, threadNum, j);
        printf("%d\n", sumXY(X,y,m,i));
        int ans = pow((m*sumXY(X,y,m,i)-sumX(X,m,i)*sumY(y,m))/((m*sumX2(X,m,i)-pow(sumX(X,m,i),2))*((m*sumY2(y,m))-pow(sumY(y,m),2))),0.5);
        printf("%d\n", ans);
        v[i+threadNum*j] =  ans;
    }
    pthread_exit(NULL); 
}


// void* testThread(void* argMatrix, void* argSize) {
//     int** matrix = (int**)argMatrix;
//     int size = (int*)argSize
// }

// void* testThread(void* argsTemp) {
//     // int** matrix = (int**)argMatrix;
//     // int size = (int*)size;
//     // printf("%d\n", *(int*)argSize);
//     args_st* args = (args_st*)argsTemp;
//     int* y = (int*)y;
//     for (int i=0; i<args->size; i++) printf("%d ", args->y[i]);
// }

int*** splitMatrix(int** matrix, int numOfThreads, int size) {
    int*** temp = (int***)malloc(sizeof(int**)*numOfThreads);
    int rowSize = size/numOfThreads;
    for(int i=0; i<numOfThreads; i++) {
        temp[i] = (int**)malloc(sizeof(int*)*size);
        for(int j=0; j<size; j++) {
            temp[i][j] = (int*)malloc(sizeof(int)*size);
            for (int k=0; k<rowSize; k++) temp[i][j][k] = matrix[j][k+rowSize*i];
        }
    }
    return temp;
}

void printSubmatrices(int*** subMatrices, int size, int numOfThreads) {
    int rowSize = size / numOfThreads;
    printf("\nPRINTING SUBMATRICES\n");
    for(int i=0; i<numOfThreads; i++) {
        printf("SUBMATRIX %d\n", i+1);
        for(int j=0; j<size; j++) {
            for (int k=0; k<rowSize; k++) printf("%d ", subMatrices[i][j][k]);
            printf("\n");
        }
        printf("\n");
    }
}

int** readMatrix(int size) {
    int** temp = (int**)malloc(sizeof(int*)*size);
    for(int i=0; i<size; i++){
        temp[i] = (int*)malloc(sizeof(int)*size);
        for (int j=0; j<size; j++) temp[i][j] = i*size+j;
    }
    return temp;
}

int* readY(int size) {
    int* temp = (int*)malloc(sizeof(int)*size);
    for(int i=0; i<size; i++) temp[i] = i*i;
    return temp;
}

int main() {
    int size = 5;
    int numOfThreads = 1;
    int* v = (int*)malloc(sizeof(int)*size);

    // int** matrix = generateRandomMatrix(size);
    int** matrix = readMatrix(size);
    int*** subMatrices = splitMatrix(matrix, numOfThreads, size);

    printf("PRINTING THE MATRIX\n");
    for(int i=0; i<size; i++) {
        for(int j=0; j<size; j++) printf("%d ", matrix[i][j]);
        printf("\n");
    }
    printSubmatrices(subMatrices, size, numOfThreads);


    // int* y = generateRandomY(size);
    int* y = readY(size);
    printf("PRINTING THE Y\n");
    for(int i=0; i<size; i++) printf("%d ", y[i]);
    printf("\n");

    pthread_t* tid = (pthread_t*)malloc(sizeof(pthread_t)*numOfThreads);
    args_st *argsArray = (args_st*)malloc(sizeof(args_st)*numOfThreads);


    clock_t t; 
    t = clock(); 

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
    
    t = clock() - t; 
    double time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds 
 
    printf("time: %f seconds\n", time_taken); 

    printf("Answer\n");
    for(int i=0; i<size; i++) printf("%d ", v[i]);
    printf("\n");

    return 0;
}



// https://www.socscistatistics.com/tests/pearson/default2.aspx