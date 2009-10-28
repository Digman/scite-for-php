#include <stdio.h>
#include "test.h"

#define HELLO 123456

typedef struct {
    const char *name[];
    int age;
} user_t;
/**
 * 这是文档注释
 * @author Jingcheng Zhang
 * @file test.c
 */
int main(int argc, const char **argv) {
    // 这是单行注释
    for (i = 0; i < argc; i++) {
        /*
         * 这是多行注释
         */
        printf("%s\n", argv[i]);
        break;
    }
    return 0;
}
