#ifndef HEADER_OPT_H
#define HEADER_OPT_H

typedef struct options_st {
    long procs;                 // 进程数
    long duration;              // 测试时间
} Options;

/* 处理参数 */
int process_options(int argc, char *argv[], Options *opt);

#endif /* HEADER_OPT_H */
