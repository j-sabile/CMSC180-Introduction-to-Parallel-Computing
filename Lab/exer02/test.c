#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

// int* generateRandomMatrix() {

// }


int randX() {
    return 4;
    // int* temp = (int*)malloc(sizeof(int));
    // *temp = 2;
    // return temp;
}

int* rand2X() {
    int* temp = (int*)malloc(sizeof(int)*4);

    for(int i=0; i<4; i++) temp[i] = i*i;
    return temp;
}

int main() {
    // printf("Test\n");
    // int x = randX();
    // printf("%d\n", x);

    // int* x2 = rand2X();
    // printf("%d %d %d %d\n", x2[0], x2[1], x2[2], x2[3]);
    
    int test = 8/3;
    printf("%d\n", test);
    return 0;
}