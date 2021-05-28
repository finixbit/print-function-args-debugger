#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void testcase1(int data1) {
}

void testcase2(int data1, char data2) {
}

void testcase3(int data1, char* data2) {
}

void testcase4(int data1, char* data2, char* data3) {
}

int main(int argc, char* argv[]) {
    testcase1(555);

    testcase2(555, 'A');

    char* testcase_data3 = (char*)malloc(sizeof(char)*10);
    strcpy(testcase_data3, "hello world");
    testcase3(555, testcase_data3);

    char* testcase_data4 = (char*)malloc(sizeof(char)*10);
    strcpy(testcase_data4, "hello world");
    char* testcase_data5 = (char*)malloc(sizeof(char)*10);
    strcpy(testcase_data5, "hello universe");
    testcase4(555, testcase_data4, testcase_data5);

    return 0;
}
