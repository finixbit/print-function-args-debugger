#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct {
    int value;
    char name;
} data1_t;

typedef struct {
    char name;
    int value;
} data2_t;

typedef struct {
    int value;
    char* name;
} data3_t;

typedef struct {
    char* name;
    int value;
} data4_t;


void testcase1(data1_t* param) {
}

void testcase2(data2_t* param) {
}

void testcase3(data3_t* param) {
}

void testcase4(data4_t* param) {
}


int main(int argc, char* argv[]) {
    data1_t d1;
    d1.name = 'A';
    d1.value = 555;
    testcase1(&d1);

    data2_t d2;
    d2.value = 555;
    d2.name = 'A';
    testcase2(&d2);

    data3_t d3;
    d3.value = 444;
    d3.name = (char*)malloc(sizeof(char)*10);
    strcpy(d3.name, "hello world");
    testcase3(&d3);

    data4_t d4;
    d4.name = (char*)malloc(sizeof(char)*10);
    strcpy(d4.name, "hello world");
    d4.value = 444;
    testcase4(&d4);

    return 0;
}
