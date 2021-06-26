#include "common.h"

#include <stdio.h>      // stderr
#include <stdarg.h>     // va_start, vfprintf, va_end
#include <stdlib.h>     // exit

void fail_if(bool condition, const char *fmt, ...) {
    if (condition) {
        va_list args;
        va_start(args, fmt);
        vfprintf(stderr, fmt, args);
        va_end(args);
        exit(1);
    }
}
void fail(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    exit(1);
}

void mylog(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}
