#include <stdio.h>      // printf, fprintf
#include <sys/wait.h>   // wait
#include <sys/types.h>  // getpid, wait
#include <limits.h>     // LONG_MIN, LONG_MAX, ULLONG_MAX
#include <unistd.h>     // getpid
#include <signal.h>     // sigaction, SIGLARM
#include "multi-process.h"

int process_index = 0;      // 进程索引编号
int isStop = 0;             // 用于标记测试终止

void handle_signal_child(int sigNum)
{
    if (sigNum == SIGALRM) {
        isStop = 1;
    }
}

/* 实际业务测试函数 */
void doTest() {
    unsigned long long i = 0;
    for(; i < ULLONG_MAX && !isStop; ++i) {
        /* Do some work */
    }
    printf("process [pid = %6u] result: %llu\n", getpid(), i);
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

    while(isParent && i < opt.procs) {
        pid =  fork();
        if(pid == -1) {         /* error */
            fprintf(stderr, "fork failed %d\n", pid);
            return -1;
        }
        else if(pid == 0) {     /* child */
            process_index = i;
            isParent = 0;
        }
        else {                  /* parent */
        }
        ++i;
    }
    if(isParent) {
        for(i =0 ; i < opt.procs; ++i) {
            pid = wait(&wstatus);                       // 等待子进程结束
            printf("process [pid = %6d] exit\n", pid);
        }
    }
    else {
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
        doTest();
        return 0;       /* child finished work */
    }

    printf("Hello World!\n");
    return 0;
}

