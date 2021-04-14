//
// Created by andre on 3/31/2021.
//
#include "types.h"
#include "user.h"

void print(char *str, int *ptr) {
    printf(1, "%s at 0x%p is %d\n", str, ptr, *ptr);
}

void test_normal() {
    int pid = fork();
    if(pid == 0) {
        printf(1, "Test Normal\n");
        int *ptr = malloc(sizeof(int));
        *ptr = 1;
        print(" - Value before", ptr);
        *ptr = 2;
        print(" - Value after", ptr);
        exit();
    }
    wait();
}

void test_protected() {
    int pid = fork();
    if(pid == 0) {
        printf(1, "Test Protected\n");
        int *ptr = malloc(sizeof(int));
        *ptr = 1;
        mprotect((void *) 0xB000, 1);
        print(" - Value before", ptr);
        *ptr = 2;
        print(" - Value after", ptr);
        exit();
    }
    wait();
}

void test_unprotected() {
    int pid = fork();
    if(pid == 0) {
        printf(1, "Test Unprotected\n");
        int *ptr = malloc(sizeof(int));
        *ptr = 1;

        mprotect((void *) 0xB000, 1);
        munprotect((void *) 0xB000, 1);

        print(" - Value before", ptr);
        *ptr = 2;
        print(" - Value after", ptr);
        exit();
    }
    wait();
}

void test_fork() {
    int *ptr = malloc(sizeof(int));
    *ptr = 1;
    mprotect((void *) 0xB000, 1);

    int pid = fork();
    if(pid == 0) {
        printf(1, "Test Fork\n");
        print(" - Value before", ptr);
        *ptr = 2;
        print(" - Value after", ptr);
        exit();
    }
    wait();
}

int main() {

    test_normal();
    test_protected();
    test_unprotected();
    test_fork();

    exit();
}