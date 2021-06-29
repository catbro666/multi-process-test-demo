#ifndef HEADER_COMMON_H
#define HEADER_COMMON_H
#include <stdbool.h>    // bool, true, false

typedef void (*cleanup) (void *);
void fail_if(bool condition, const char *fmt, ...);
void fail(const char *fmt, ...);
void mylog(const char *fmt, ...);
void fail_clean_if(bool condition, cleanup clean, void *param, const char *fmt, ...);


#endif /* HEADER_COMMON_H */
