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

#include <string.h>
#include <sys/socket.h>

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
    printf("\nPEARSON CORRELATION COEFFICIENT\n");
    for(int i=0; i<size; i++) printf("%lf ", v[i]);
    printf("\n");
}

void pearson_cor_basic(int** X, int* y, float* v, int m, int n) {
    long resSumY = sumY(y,m);
    long resSumY2 = sumY2(y,m);
    long resSumY_2 = pow(resSumY,2);
    for(int i=0; i<n; i++) {
        v[i] = (m*sumXY(X,y,m,i)-sumX(X,m,i)*sumY(y,m))/pow((m*sumX2(X,m,i)-pow(sumX(X,m,i),2))*((m*resSumY2)-resSumY_2),0.5);
    }
}

int send_data(int sockfd, const void *buffer, size_t length, size_t element_size) {
    const char *ptr = (const char *)buffer;
    size_t total_sent = 0;

    while (total_sent < length * element_size) {
        ssize_t bytes_sent = send(sockfd, ptr + total_sent, (length * element_size) - total_sent, 0);
        if (bytes_sent <= 0) {
            if (bytes_sent == 0) {
                fprintf(stderr, "Connection closed by peer\n");
            } else {
                fprintf(stderr, "Error sending data: %s\n", strerror(errno));
            }
            return -1;
        }
        total_sent += bytes_sent;
    }

    return 0;
}

int receive_data(int sockfd, void *buffer, size_t length, size_t element_size) {
    char *ptr = (char *)buffer;
    size_t total_received = 0;

    while (total_received < length * element_size) {
        ssize_t bytes_received = recv(sockfd, ptr + total_received, (length * element_size) - total_received, 0);
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                fprintf(stderr, "Connection closed by peer\n");
            } else {
                fprintf(stderr, "Error receiving data: %s\n", strerror(errno));
            }
            return -1;
        }
        total_received += bytes_received;
    }

    return 0;
}

void slaveFunc(int connfd, int* n) {
    int m; 
    receive_data(connfd, &m, 1, sizeof(int));
    receive_data(connfd, n, 1, sizeof(int));
    // read(connfd, &m, sizeof(int));
    // read(connfd, n, sizeof(int));
    // printf("m=%d n=%d\n", m, *n);

    int* y = (int*)malloc(sizeof(int)*m);
    float* v = (float*)malloc(sizeof(float)*(int)*n);
    int** matrix = (int**)malloc(sizeof(int*)*m);
    for(int i=0; i<m; i++) matrix[i] = (int*)malloc(sizeof(int)*(int)*n);
    
    receive_data(connfd, y, m, sizeof(int));
    for(int i=0; i<m; i++) receive_data(connfd, matrix[i], (int)*n, sizeof(int));
    int ack = 1;
    send_data(connfd, &ack, 1, sizeof(int));
    
    // for(int i=0; i<(int)*n; i++) printf("%d ", matrix[0][i]);

    // read(connfd, y, sizeof(int)*m);
    // for(int i=0; i<m; i++) read(connfd, matrix[i], sizeof(int)*(int)*n);
    // int ack = 1;
    // write(connfd, &ack, sizeof(int));
    printf("Received the matrix from the master!\n");

    // pearson_cor_basic(matrix, y, v, m, (int)*n);
    for (int i=0; i<m; i++) free(matrix[i]);
    free(matrix);
    // printf("answers:\n");
    // for(int i=0; i<(int)*n; i++) printf("%lf ", v[i]);
    // printf("\n");

    // return v;
}

void slaveFunc2(int sockfd, float* v, int len, int* slaveNumber) {
    // printf("sending: %d %d ", (int)*slaveNumber, len);
    send_data(sockfd, slaveNumber, 1, sizeof(int));
    send_data(sockfd, &len, 1, sizeof(int));
    send_data(sockfd, v, len, sizeof(int));

    // write(sockfd, slaveNumber, sizeof(int));
    // write(sockfd, &len, sizeof(int));
    // write(sockfd, v, sizeof(float)*len);
}

void masterFunc(int sockfd, int* y, int m, int n, int** subMatrix) {
    int ack;
    send_data(sockfd, &m, 1, sizeof(int));
    send_data(sockfd, &n, 1, sizeof(int));

    // write(sockfd, &m, sizeof(int));
    // write(sockfd, &n, sizeof(int));

    send_data(sockfd, y, m, sizeof(int));
    for (int i=0; i<m; i++) send_data(sockfd, subMatrix[i], n, sizeof(int));
    receive_data(sockfd, &ack, 1, sizeof(int));
    printf("Successfully sent the matrix!\n");


    // write(sockfd, y, sizeof(int)*m);
    // for (int i=0; i<m; i++) write(sockfd, subMatrix[i], sizeof(int)*n);
    // read(sockfd, &ack, sizeof(int));
    // printf("Successfully sent the matrix!\n");
}

void masterFunc2(int connfd, float* v) {
    int slaveNumber, len;

    receive_data(connfd, &slaveNumber, 1, sizeof(int));
    receive_data(connfd, &len, 1, sizeof(int));
    float* temp = (float*)malloc(sizeof(float)*len);
    receive_data(connfd, temp, len, sizeof(int));

    // read(connfd, &slaveNumber, sizeof(int));
    // read(connfd, &len, sizeof(int));
    // float* temp = (float*)malloc(sizeof(float)*len);
    // read(connfd, temp, sizeof(float)*len);
    
    
    // printf("len=%d, sl#=%d\n", len, slaveNumber);
    // printf("temp=");
    // for(int i=0; i<len; i++) printf("%lf ", temp[i]);
    // printf("\n");
    for (int i=0; i<len; i++) {
        // printf("%d ", slaveNumber*len+i);
        v[slaveNumber*len+i] = (float)temp[i];
    }
    // printf("\n");
    free(temp);
}

int createSocket(struct sockaddr_in* servaddr, int port, const char* ip_addr, bool isClient) {
    int sockfd;

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Socket creation failed...\n");
        exit(0);
    }
    // else printf("Socket successfully created!\n"); VERBB
    bzero(servaddr, sizeof(servaddr));

    // // Assign IP and PORT
    servaddr->sin_family = AF_INET;
    if (isClient) {servaddr->sin_addr.s_addr = inet_addr(ip_addr); printf("%s %d\n", ip_addr, port);} // Convert IP address string to binary form
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
    // else printf("Socket successfully binded..\n"); VERBB
}

int listenSocket(int sockfd, struct sockaddr_in* cli, const char* s1, const char* s2) {
    if ((listen(sockfd, 5)) != 0) { 
        printf("Listen failed...\n"); 
        exit(0); 
    } 
    else printf("%s\n", s1);

    int len = sizeof(*cli); 
    int connfd = accept(sockfd, (SA*)cli, &len); 
    if (connfd < 0) { 
        printf("server accept failed...\n"); 
        exit(0); 
    } 
    else printf("%s\n", s2);
    return connfd; 
}

void connectSocket(int sockfd, struct sockaddr_in* servaddr, char* ipAddress, int port, const char* s) {
    srand(time(NULL));
    printf("Connecting to %s:%d...\n", ipAddress, port);
    while (connect(sockfd, (SA*)servaddr, sizeof(*servaddr)) != 0) {sleep(rand()%3);}
    printf("%s %s:%d...\n", s, ipAddress, port);
}

bool askBool(const char* prompt) {
    char temp;
    printf("%s [Y/N]: ", prompt);
    scanf(" %c", &temp);
    if (temp == 'Y') return true;
    return false;
}

void runInCore(int core) {
    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);
    CPU_SET(core, &cpu_set);
    int ret = sched_setaffinity(0, sizeof(cpu_set_t), &cpu_set);
    if (ret != 0) perror("sched_setaffinity");
    printf("Running on core %d\n", core);
}



int main(int argc, char *argv[]) {
    // reading the config
    int numberOfSlaves, hostPort;
    int numOfCores = sysconf(_SC_NPROCESSORS_ONLN);
    char* hostIp = (char*)malloc(sizeof(char)*16);
    FILE* fp = fopen("jerico-config.txt", "r");
    if (fp == NULL) {
        printf("Failed to open the config file...\n");
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

    bool isCoreAffine = askBool("Core-Affine?");
    // bool isCoreAffine = true;

    printf("s [ 0:Master / 1:Slave ]: ");
    scanf(" %c", &s);

    if (s == '0') {
        printf("\n==== MASTER ====\n");

        if (isCoreAffine) runInCore(1);

        printf("n: ");
        scanf("%d", &n);
        // n = 20000;
        // bool verbose = askBool("Print Matrix?");
        bool verbose = false;

        int** matrix = generateRandomMatrix(n);
        int* y = generateRandomY(n);
        if(verbose) printY(y, n);
        int*** subMatrices = splitMatrix(matrix, numberOfSlaves, n);
        if(verbose) printSubmatrices(subMatrices, numberOfSlaves, n/numberOfSlaves, n);
        for (int i=0; i<n; i++) free(matrix[i]);
        free(matrix);

        struct timespec start;
        clock_gettime(CLOCK_MONOTONIC, &start);

        for (int i=0; i<numberOfSlaves; i++) {
            sockfd = createSocket(&servaddr, slavesPort[i], slavesIp[i], true);
            connectSocket(sockfd, &servaddr, slavesIp[i], slavesPort[i], "Sending the matrix to the slave");
            masterFunc(sockfd, y, n, n/numberOfSlaves, subMatrices[i]);
        	close(sockfd);
            for (int j=0; j<n; j++) free(subMatrices[i][j]);
            free(subMatrices[i]);
        }

        printf("\nSuccessfully sent all matrices to the slaves!\n\n");

        // float* v = (float*)malloc(sizeof(float)*n);
        // sockfd = createSocket(&servaddr, hostPort, INADDR_ANY, false);
        // bindSocket(sockfd, &servaddr);
        // for (int i=0; i<numberOfSlaves; i++) {
        //     char s[70];
        //     sprintf(s, "Waiting for the slave%d to send the computed pearson...", i);
        //     connfd = listenSocket(sockfd, &cli, s, "Receiving the pearson from the slave!");
        //     masterFunc2(connfd, v);
        // }
        // close(sockfd);

        struct timespec end;
        clock_gettime(CLOCK_MONOTONIC, &end);
        // if(verbose) printResult(v, n);
        printf("\ntime: %f seconds\n", (end.tv_sec-start.tv_sec) + (end.tv_nsec-start.tv_nsec) / 1000000000.0); 

        // printf("\n=== Compiled ===\n");
        // for (int i=0; i<n; i++) printf("%lf ", v[i]);

    } else if (s == '1') {
        printf("\n==== SLAVE ====\n");

        printf("Slave Number [0-%d]: ", numberOfSlaves-1);
        scanf("%d", &slaveNumber);

        if (isCoreAffine) runInCore((slaveNumber+1)%numOfCores);
        
        sockfd = createSocket(&servaddr, slavesPort[slaveNumber], INADDR_ANY, false);
        bindSocket(sockfd, &servaddr);
        connfd = listenSocket(sockfd, &cli, "Waiting for the master to send the matrix...", "Receiving the matrix from the master!");
        int* n = (int*)malloc(sizeof(int));
        slaveFunc(connfd, n); 
        // close(connfd);

        // printf("\nSolved the pearson of the matrix!\n\n");
        
        // sockfd = createSocket(&servaddr, hostPort, hostIp, true);
        // connectSocket(sockfd, &servaddr, hostIp, hostPort, "Sending the pearson to the master");
        // slaveFunc2(sockfd, v, (int)*n, &slaveNumber);
        // printf("Successfully sent the pearson to the master!\n");
        // close(sockfd);

    } else { printf("Invalid s input!"); }

    return 0;
}

// scp user@10.0.4.108:~/Desktop/jerico .
// scp user@10.0.4.108:~/Desktop/jerico-config.txt .

// 10.0.5.27 drone01.swarm.ics.uplb.edu.ph drone01 computeling01
// 10.0.5.30 drone02.swarm.ics.uplb.edu.ph drone02 computeling02
// 10.0.5.32 drone03.swarm.ics.uplb.edu.ph drone03 computeling03
// 10.0.4.47 drone04.swarm.ics.uplb.edu.ph drone04 computeling04

// 10.0.4.46 drone05.swarm.ics.uplb.edu.ph drone05 computeling05
// 10.0.4.45 drone06.swarm.ics.uplb.edu.ph drone06 computeling06
// 10.0.4.88 drone07.swarm.ics.uplb.edu.ph drone07 computeling07
// 10.0.4.94 drone08.swarm.ics.uplb.edu.ph drone08 computeling08

// 10.0.4.84 drone09.swarm.ics.uplb.edu.ph drone09 computeling09 XXX 1
// 10.0.4.80 drone10.swarm.ics.uplb.edu.ph drone10 computeling10
// 10.0.4.87 drone11.swarm.ics.uplb.edu.ph drone11 computeling11 XXX 2
// 10.0.4.81 drone12.swarm.ics.uplb.edu.ph drone12 computeling12 

// 10.0.4.225 drone13.swarm.ics.uplb.edu.ph drone13 computeling13 XXX 3
// 10.0.4.227 drone14.swarm.ics.uplb.edu.ph drone14 computeling14 
// 10.0.4.228 drone15.swarm.ics.uplb.edu.ph drone15 computeling15 XXX 4
// 10.0.5.123 drone16.swarm.ics.uplb.edu.ph drone16 computeling16

// 10.0.5.166 drone17.swarm.ics.uplb.edu.ph drone17 computeling17 XXX 5
// 10.0.5.168 drone18.swarm.ics.uplb.edu.ph drone18 computeling18 XXX 6
// 10.0.4.119 drone19.swarm.ics.uplb.edu.ph drone19 computeling19 XXX 7
// 10.0.4.120 drone20.swarm.ics.uplb.edu.ph drone20 computeling20 XXX 8

// 10.0.4.124 drone21.swarm.ics.uplb.edu.ph drone21 computeling21
// 10.0.5.14  drone22.swarm.ics.uplb.edu.ph drone22 computeling22
// 10.0.5.9   drone23.swarm.ics.uplb.edu.ph drone23 computeling23
// 10.0.5.11  drone24.swarm.ics.uplb.edu.ph drone24 computeling24

// 10.0.5.143 drone25.swarm.ics.uplb.edu.ph drone25 computeling25
// 10.0.5.144 drone26.swarm.ics.uplb.edu.ph drone26 computeling26
// 10.0.5.145 drone27.swarm.ics.uplb.edu.ph drone27 computeling27
