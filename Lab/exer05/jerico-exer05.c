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

// function to send large data
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

// function to receive large data
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

    int* y = (int*)malloc(sizeof(int)*m);
    float* v = (float*)malloc(sizeof(float)*(int)*n);
    int** matrix = (int**)malloc(sizeof(int*)*m);
    for(int i=0; i<m; i++) matrix[i] = (int*)malloc(sizeof(int)*(int)*n);
    
    receive_data(connfd, y, m, sizeof(int));
    for(int i=0; i<m; i++) receive_data(connfd, matrix[i], (int)*n, sizeof(int));
    int ack = 1;
    send_data(connfd, &ack, 1, sizeof(int));
    
    printf("Received the matrix from the master!\n");

    for (int i=0; i<m; i++) free(matrix[i]);
    free(matrix);
}

void masterFunc(int sockfd, int* y, int m, int n, int** subMatrix) {
    int ack;
    send_data(sockfd, &m, 1, sizeof(int));
    send_data(sockfd, &n, 1, sizeof(int));

    send_data(sockfd, y, m, sizeof(int));
    for (int i=0; i<m; i++) send_data(sockfd, subMatrix[i], n, sizeof(int));
    receive_data(sockfd, &ack, 1, sizeof(int));
    printf("Successfully sent the matrix!\n");
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

    printf("s [ 0:Master / 1:Slave ]: ");
    scanf(" %c", &s);

    if (s == '0') {
        printf("\n==== MASTER ====\n");

        if (isCoreAffine) runInCore(1);

        printf("n: ");
        scanf("%d", &n);

        int** matrix = generateRandomMatrix(n);
        int* y = generateRandomY(n);
        int*** subMatrices = splitMatrix(matrix, numberOfSlaves, n);
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

        struct timespec end;
        clock_gettime(CLOCK_MONOTONIC, &end);
        printf("\ntime: %f seconds\n", (end.tv_sec-start.tv_sec) + (end.tv_nsec-start.tv_nsec) / 1000000000.0); 


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

    } else { printf("Invalid s input!"); }

    return 0;
}
