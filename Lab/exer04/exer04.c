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

#include <arpa/inet.h> // inet_addr()
#include <unistd.h> // read(), write(), close()
#include <strings.h> // bzero()
#include <netdb.h> 
#include <netinet/in.h> 

#define SA struct sockaddr 
#define MAX 80 
#define PORT 8082

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

void func(int sockfd) {

}

void pearson_cor_basic(int** X, int* y, float* v, int m, int n) {
    long resSumY = sumY(y,m);
    long resSumY2 = sumY2(y,m);
    long resSumY_2 = pow(resSumY,2);
    for(int i=0; i<n; i++) {
        v[i] = (m*sumXY(X,y,m,i)-sumX(X,m,i)*sumY(y,m))/pow((m*sumX2(X,m,i)-pow(sumX(X,m,i),2))*((m*resSumY2)-resSumY_2),0.5);
    }
}

void slaveFunc(int connfd) {
    int m, n; 
    read(connfd, &m, sizeof(int));
    read(connfd, &n, sizeof(int));

    int* y = (int*)malloc(sizeof(int)*m);
    float* v = (float*)malloc(sizeof(float)*n);
    int** matrix = (int**)malloc(sizeof(int*)*m);
    for(int i=0; i<m; i++) matrix[i] = (int*)malloc(sizeof(int)*n);
    
    read(connfd, y, sizeof(int)*m);
    for(int i=0; i<m; i++) read(connfd, matrix[i], sizeof(int)*n);
    int ack = 1;
    write(connfd, &ack, sizeof(int));

    pearson_cor_basic(matrix, y, v, m, n);
    printf("answers:\n");
    for(int i=0; i<n; i++) printf("%lf ", v[i]);

    write(connfd, v, sizeof(float)*n);
}

void masterFunc(int sockfd, int* y, int m, int n, int** subMatrix) {
    int ack;
    write(sockfd, &m, sizeof(int));
    write(sockfd, &n, sizeof(int));

    write(sockfd, y, sizeof(int)*m);
    for (int i=0; i<m; i++) write(sockfd, subMatrix[i], sizeof(int)*n);
    read(sockfd, &ack, sizeof(int));
    printf("Successfully sent data!\n");

    // read(sockfd, );
}

int createSocket(struct sockaddr_in* servaddr, int port, const char* ip_addr) {
    int sockfd;

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Socket creation failed...\n");
        exit(0);
    }
    else printf("Socket successfully created!\n");
    bzero(servaddr, sizeof(servaddr));

    // // Assign IP and PORT
    servaddr->sin_family = AF_INET;
    if (ip_addr) servaddr->sin_addr.s_addr = inet_addr(ip_addr); // Convert IP address string to binary form
    else servaddr->sin_addr.s_addr = htonl(INADDR_ANY); // Use INADDR_ANY for any available IP
    servaddr->sin_port = htons(port);

    return sockfd;
}

void bindSocket(int sockfd, struct sockaddr_in* servaddr) {
    // Binding newly created socket to given IP and verification 
    if ((bind(sockfd, (SA*)servaddr, sizeof(*servaddr))) != 0) { 
        printf("socket bind failed...\n"); 
        exit(0); 
    } 
    else printf("Socket successfully binded..\n"); 
}

int listenSocket(int sockfd, struct sockaddr_in* cli) {
    if ((listen(sockfd, 5)) != 0) { 
        printf("Listen failed...\n"); 
        exit(0); 
    } 
    else printf("Server listening..\n");

    int len = sizeof(*cli); 
    int connfd = accept(sockfd, (SA*)cli, &len); 
    if (connfd < 0) { 
        printf("server accept failed...\n"); 
        exit(0); 
    } 
    else printf("server accept the client...\n");
    return connfd; 
}

bool askVerbose() {
    char temp;
    printf("Print matrix? [Y/N]: ");
    scanf(" %c", &temp);
    if (temp == 'Y') return true;
    return false;
}

int main(int argc, char *argv[]) {
    
    // reading the config
    int numberOfSlaves, hostPort;
    char* hostIp = (char*)malloc(sizeof(char)*16);
    FILE* fp = fopen("config.txt", "r");
    if (fp == NULL) {
        printf("Failed to open the config file...");
        exit(0);
    }
    fscanf(fp, "%d\n", &numberOfSlaves);
    fscanf(fp, "%s\n", hostIp);
    fscanf(fp, "%d\n", &hostPort);
    char** slavesIp = (char**)malloc(sizeof(char*)*numberOfSlaves);
    for(int i=0; i<numberOfSlaves; i++) slavesIp[i] = (char*)malloc(sizeof(char)*16);
    int* slavesPort = (int*)malloc(sizeof(int)*numberOfSlaves);
    for(int i=0; i<numberOfSlaves; i++) {
        fscanf(fp, "%s\n", slavesIp[i]);
        fscanf(fp, "%d\n", &slavesPort[i]);
    }

    int size, n, sockfd, connfd, len, slaveNumber;
    char temp, s;
	struct sockaddr_in servaddr, cli; 

    printf("s [0/1]: ");
    scanf(" %c", &s);

    if (s == '0') {
        printf("\n==== MASTER ====\n");

        printf("n: ");
        scanf("%d", &n);
        bool verbose = askVerbose();

        float* v = (float*)malloc(sizeof(float)*n);
        int** matrix = generateRandomMatrix(n);
        int* y = generateRandomY(n);
        if(verbose) printY(y, n);
        int*** subMatrices = splitMatrix(matrix, numberOfSlaves, n);
        if(verbose) printSubmatrices(subMatrices, numberOfSlaves, n/numberOfSlaves, n);

        for (int i=0; i<numberOfSlaves; i++) {

            sockfd = createSocket(&servaddr, slavesPort[i], slavesIp[i]);

            // connect the client socket to server socket
            if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr))
                != 0) {
                printf("connection to %s %d failed...\n", slavesIp[i], slavesPort[i]);
                exit(0);
            }
            else printf("connected to %s %d..\n", slavesIp[i], slavesPort[i]);

            masterFunc(sockfd, y, n, n/numberOfSlaves, subMatrices[i]);

        	close(sockfd);
        }


    } else if (s == '1') {
        printf("\n==== SLAVE ====\n");

        printf("Slave Number [0-%d]: ", numberOfSlaves-1);
        scanf("%d", &slaveNumber);

        sockfd = createSocket(&servaddr, slavesPort[slaveNumber], INADDR_ANY);
        bindSocket(sockfd, &servaddr);
        connfd = listenSocket(sockfd, &cli);

        slaveFunc(connfd); 
        // receive data
        // send ack
        // compute data
        // send back data

        close(sockfd); 

    } else { printf("Invalid s input!"); }

    // printf("Size: ");
    // scanf("%d", &size);
    // printf("# of threads: ");
    // scanf("%d", &numOfThreads);

    // printf("Print matrix? [Y/N]: ");
    // scanf(" %c", &temp);
    // if (temp == 'Y') { verbose = true; }

    // float* v = (float*)malloc(sizeof(float)*size);
    // int** matrix = generateRandomMatrix(size);
    // int* y = generateRandomY(size);
    // if(verbose) printY(y, size);

    // pthread_t* tid = (pthread_t*)malloc(sizeof(pthread_t)*numOfThreads);
    
    // int*** subMatrices = splitMatrix(matrix, numOfThreads, size);
    // if(verbose) printSubmatrices(subMatrices, numOfThreads, size/numOfThreads, size);

    // args_st *argsArray = (args_st*)malloc(sizeof(args_st)*numOfThreads);

    // struct timespec start;
    // clock_gettime(CLOCK_MONOTONIC, &start);

    // long resSumY = sumY(y,size);
    // long resSumY2 = sumY2(y,size);
    // long resSumY_2 = pow(resSumY,2);
    // for(int i=0; i<numOfThreads; i++) {
    //     argsArray[i].X = subMatrices[i];
    //     argsArray[i].y = y;
    //     argsArray[i].resSumY = resSumY;
    //     argsArray[i].resSumY2 = resSumY2;
    //     argsArray[i].resSumY_2 = resSumY_2;
    //     argsArray[i].colSize = size;
    //     argsArray[i].rowSize = size/numOfThreads;
    //     argsArray[i].threadNum = i;
    //     argsArray[i].numCores = num_cores;
    //     argsArray[i].v = v;
    //     pthread_create(&tid[i], NULL, pearson_cor, (void*)&argsArray[i]);
    // }

    // for(int i=0; i<numOfThreads; i++) pthread_join(tid[i], NULL);
    
    // struct timespec end;
    // clock_gettime(CLOCK_MONOTONIC, &end);
    // if(verbose) printResult(v, size);
    // printf("time: %f seconds\n", (end.tv_sec-start.tv_sec) + (end.tv_nsec-start.tv_nsec) / 1000000000.0); 
    return 0;
}