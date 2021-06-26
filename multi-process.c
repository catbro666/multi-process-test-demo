#include <stdio.h>      // printf, fprintf
#include <sys/wait.h>   // wait
#include <sys/types.h>  // getpid, wait
#include <sys/ipc.h>    // shmget, shmctl
#include <sys/shm.h>    // shmget, shmctl
#include <signal.h>     // sigaction, SIGLARM
#include <limits.h>     // LONG_MIN, LONG_MAX, ULLONG_MAX
#include <errno.h>      // errno
#include <unistd.h>     // getpid
#include <string.h>     // memset
#include "multi-process.h"

typedef struct param_st {   // 自定义测试参数
    long index;
} Param;

typedef struct result_st {   // 自定义测试结果
    unsigned long long count;
} Result;

int isStop = 0;             // 用于标记测试终止
Options opt;                // 命令行选项
int shmid;                  // 共享内存id
Result *shm = NULL;         // 共享内存地址，用于存放测试结果
Result res_total;
Result res_last;

void handle_signal_child(int sigNum)
{
    if (sigNum == SIGALRM) {
        isStop = 1;
    }
}

void handle_signal_parent(int sigNum)
{
    if (sigNum == SIGALRM) {
        /* DO REAL-TIME STATISTICS */
        memset(&res_total, 0, sizeof(Result));
        for (long i = 0; i < opt.procs; ++i) {
            res_total.count += shm[i].count;
        }
        fprintf(stderr, "total count %12llu, average %12.0lf\n",
                res_total.count, (res_total.count - res_last.count)
                / (double)opt.interval);
        memcpy(&res_last, &res_total, sizeof(Result));
        /* DO REAL-TIME STATISTICS */
        alarm(opt.interval);
    }
}

/* 实际业务测试函数 */
void doTest(void *param) {
    unsigned long long i = 0;
    Param *pa = (Param *)param;
    for (; i < ULLONG_MAX && !isStop; ++i) {
        /* DO YOUR WORK */
        ++shm[pa->index].count;
        /* DO YOUR WORK */
    }
    fprintf(stderr, "process [pid = %6u] result: %llu\n",
           getpid(), shm[pa->index].count);
}

int main(int argc, char *argv[]) {
    int rv = 0;
    long i = 0;
    int proc_index = 0;
    int isParent = 1;
    int wstatus = 0;
    pid_t pid = 0;
    struct sigaction act_child;
    struct sigaction act_parent;
    struct sigaction act_quit;
    struct sigaction act_int;

    rv = process_options(argc, argv, &opt);
    if (rv) {
        return -1;
    }

    fprintf(stderr, "\n-----------------------------Start Testing------------------------------\n\n");


    /* COMMON INIT */
    shmid = shmget(IPC_PRIVATE, sizeof(sizeof(Result) * opt.procs), 0666);
    if (-1 == shmid) {
        fprintf(stderr, "shmget() failed\n");
        return -1;
    }
    //fprintf(stderr, "shmid = %d\n", shmid);
    shm = (Result*)shmat(shmid, 0, 0);
    if ((void *) -1 == shm) {
        fprintf(stderr, "shmat() failed\n");
        return -1;
    }
    memset(shm, 0, sizeof(sizeof(Result) * opt.procs));
    /* COMMON INIT */

    while(isParent && i < opt.procs) {
        pid =  fork();
        if(pid == -1) {         /* error */
            fprintf(stderr, "fork failed %d\n", pid);
            return -1;
        }
        else if(pid == 0) {     /* child */
            isParent = 0;
            proc_index = i;     // 记录进程索引
        }
        else {                  /* parent */
        }
        ++i;
    }
    if(isParent) {
        /* PARENT INIT */
        memset(&act_parent, 0, sizeof(act_parent));
        act_parent.sa_handler = handle_signal_parent;
        act_parent.sa_flags = SA_RESTART;               // 使wait被中断时可以自动恢复 
        rv = sigaction(SIGALRM, &act_parent, NULL);     // 用于定时统计结果
        //signal(SIGALRM, handle_signal_parent);
        if (rv) {
            fprintf(stderr, "sigaction() failed\n");   
            return -1;
        }
#if 0
        act_quit.sa_handler = handle_signal_exit;
        sigemptyset(&act_quit.sa_mask);
        sigaddset(&act_quit.sa_mask, SIGQUIT);
        sigaddset(&act_quit.sa_mask, SIGINT);
        act_quit.sa_flags = SA_RESETHAND;
        rv = sigaction(SIGALRM, &act_quit, NULL);      // 捕获SIGQUIT信号，终止前释放共享内存
        if (rv) {
            fprintf(stderr, "sigaction() failed\n");
            return -1;
        }

        act_int.sa_handler = handle_signal_exit;
        sigemptyset(&act_int.sa_mask);
        sigaddset(&act_quit.sa_mask, SIGQUIT);
        sigaddset(&act_quit.sa_mask, SIGINT);
        act_quit.sa_flags = SA_RESETHAND;
        rv = sigaction(SIGALRM, &act_int, NULL);      // 捕获SIGINT信号，终止前释放共享内存
        if (rv) {
            fprintf(stderr, "sigaction() failed\n");
            return -1;
        }
#endif
        memset(&res_last, 0, sizeof(Result));
        alarm(opt.interval);
        /* PARENT INIT */
        for(i =0 ; i < opt.procs; ++i) {
            pid = wait(&wstatus);                       // 等待子进程结束
            alarm(0);                                   // 终止定时器
            if(pid == -1) {
                fprintf(stderr, "wait() failed, errno=%d\n", errno);
            }
            fprintf(stderr, "process [pid = %6d] exit\n", pid);
        }
        
        shmdt((void*)shm);
        /* 子进程退出之后自动detach了, 所以这里不需要通过IPC_STAT进行判断 */
        shmctl(shmid, IPC_RMID, 0);
    }
    else {
        /* CHILD INIT */
        Param param;
        memset(&param, 0, sizeof(Param));
        param.index = proc_index;
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

    return 0;
}

