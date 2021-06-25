#include <stdio.h>      // printf, fprintf
#include <sys/wait.h>   // wait
#include <sys/types.h>  // getpid, wait
#include <signal.h>     // sigaction, SIGLARM
#include <limits.h>     // LONG_MIN, LONG_MAX, ULLONG_MAX
#include <unistd.h>     // getpid
#include <string.h>     // memset
#include "multi-process.h"

int isStop = 0;             // 用于标记测试终止

typedef struct param_st {   // 自定义测试参数
    long long count;
} Param;

void handle_signal_child(int sigNum)
{
    if (sigNum == SIGALRM) {
        isStop = 1;
    }
}

/* 实际业务测试函数 */
void doTest(void *param) {
    unsigned long long i = 0;
    Param *pa = (Param *)param;
    for(; i < ULLONG_MAX && !isStop; ++i) {
        /* DO YOUR WORK */
        ++pa->count;
        /* DO YOUR WORK */
    }
    printf("process [pid = %6u] result: %llu\n", getpid(), pa->count);
}

int main(int argc, char *argv[]) {
    int rv = 0;
    long i = 0;
    Options opt;
    int isParent = 1;
    int wstatus = 0;
    pid_t pid = 0;
    struct sigaction act_child;

    rv = process_options(argc, argv, &opt);
    if (rv) {
        return -1;
    }

    printf("\n-----------------------------Start Testing------------------------------\n\n");

    /* COMMON INIT */
    /* COMMON INIT */

    while(isParent && i < opt.procs) {
        pid =  fork();
        if(pid == -1) {         /* error */
            fprintf(stderr, "fork failed %d\n", pid);
            return -1;
        }
        else if(pid == 0) {     /* child */
            isParent = 0;
        }
        else {                  /* parent */
        }
        ++i;
    }
    if(isParent) {
        /* PARENT INIT */
        /* PARENT INIT */
        for(i =0 ; i < opt.procs; ++i) {
            pid = wait(&wstatus);                       // 等待子进程结束
            printf("process [pid = %6d] exit\n", pid);
        }
    }
    else {
        /* CHILD INIT */
        Param param;
        memset(&param, 0, sizeof(Param));
        /* CHILD INIT */

        act_child.sa_handler = handle_signal_child;
        sigemptyset(&act_child.sa_mask);
        //sigaddset(&act_child.sa_mask, SIGQUIT);
        //sigaddset(&act_child.sa_mask, SIGTERM);
        act_child.sa_flags = SA_RESETHAND;
        rv = sigaction(SIGALRM, &act_child, NULL);      // 用于测试时间到时，通知子进程结束测试
        if (rv) {
            fprintf(stderr, "sigaction() failed\n");   
            return -1;
        }
        //signal(SIGALRM, handle_signal_child);
        alarm(opt.duration);                            // 设置测试时长
        doTest(&param);
        return 0;       /* child finished work */
    }

    printf("Hello World!\n");
    return 0;
}

