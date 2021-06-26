#ifndef HEADER_COMMON_H
#define HEADER_COMMON_H
#include <stdbool.h>    // bool, true, false

void fail_if(bool condition, const char *fmt, ...);
void fail(const char *fmt, ...);
void mylog(const char *fmt, ...);

#endif /* HEADER_COMMON_H */
