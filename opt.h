#ifndef HEADER_OPT_H
#define HEADER_OPT_H

typedef enum test{
    hash, sign, verify, enc, dec
} Test;

typedef struct options_st {
    long procs;                 // 进程数
    long duration;              // 测试时间
    long interval;              // 统计间隔
    Test test;                  // 测试类型
    long len;                   // 摘要原文长度
    const char *key;            // 密钥文件路径
    const char *cert;           // 证书文件路径
    const char *loglevel;       // 日志等级
} Options;

/* 处理参数 */
int process_options(int argc, char *argv[], Options *opt);

#endif /* HEADER_OPT_H */
