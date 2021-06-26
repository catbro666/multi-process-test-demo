#include "opt.h"    

#include <limits.h>     // LONG_MIN, LONG_MAX, ULLONG_MAX
#include <string.h>     // memset
#include <stdlib.h>     // strtol, abort
#include <stdio.h>      // fprintf
#include <getopt.h>     // geropt_long

static void ShowHelpInfo(char *name) {
    fprintf(stderr, "Usage: %s [options]\n\n", name);
    fprintf(stderr, "  Options:\n");
    fprintf(stderr, "    -p/--proc         Number of processes (default: 1)\n");
    fprintf(stderr, "    -d/--duration     Duration of test (unit: s, default: 10)\n");
    fprintf(stderr, "    -i/--interval     Interval of statisics (unit: s, default: 1)\n");
    fprintf(stderr, "    -h/--help         Show the help info\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  Example:\n");
    fprintf(stderr, "    %s -p 4 -d 30 -i 2\n", name);
    fprintf(stderr, "\n");
}

/* 处理参数 */
int process_options(int argc, char *argv[], Options *opt) {
    int c = 0;
    int option_index = 0;
    long procs = 1;
    long duration = 10;
    long interval = 1;
    /**     
     *  定义命令行参数列表，option结构的含义如下（详见 man 3 getopt）：
     *  struct option {
     *      const char *name;       // 参数的完整名称，对应命令中的 --xxx
     *      int  has_arg;           // 该参数是否带有一个值，如 –config xxx.conf
     *      int *flag;              // 一般设置为NULL
     *      int  val;               // 解析到该参数后getopt_long函数的返回值，为了方便维护，一般对应getopt_long调用时第三个参数
     *  };
     */
    static struct option arg_options[] = 
    {
        {"proc", 1, NULL, 'p'},
        {"duration", 1, NULL, 'd'},
        {"duration", 1, NULL, 'i'},
        {"help", 0, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };

    /**
     *  注意：传递给getopt_long的第三个参数对应了命令行参数的缩写形式，如-h, -v, -c等，
     *  如果字符后面带有冒号":"，则说明该参数后跟一个值，如-c xxxxxx             
     *  如果开头有冒号":"，则当一个选项缺少参数时，返回":"，否则，返回"?"
     */
    while ((c = getopt_long(argc, argv, ":p:d:i:h", arg_options, &option_index)) != -1) {
        switch (c) {
        case 'h':
            ShowHelpInfo(argv[0]);
            //fprintf(stderr,"option is -%c, optarv is %s\n", c, optarg);
            return 0;
        case 'p':
            procs = strtol(optarg, NULL, 0);
            if (procs == LONG_MIN || procs == LONG_MAX) {
                fprintf(stderr, "The number of processes (%s) is overflow\n\n", optarg);
                ShowHelpInfo(argv[0]);
                return -1;
            }
            else if (procs <= 0) {
                fprintf(stderr, "The number of processes must be > 0\n\n");
                ShowHelpInfo(argv[0]);
                return -1;
            }
            break;
        case 'd':
            duration = strtol(optarg, NULL, 0);
            if (duration == LONG_MIN || duration == LONG_MAX) {
                fprintf(stderr, "The duration of test (%s) is overflow\n\n", optarg);
                ShowHelpInfo(argv[0]);
                return -1;
            }
            else if (procs <= 0) {
                fprintf(stderr, "The duration of test must be > 0\n\n");
                ShowHelpInfo(argv[0]);
                return -1;
            }
            break;
        case 'i':
            interval = strtol(optarg, NULL, 0);
            if (interval == LONG_MIN || interval == LONG_MAX) {
                fprintf(stderr, "The interval of statistics (%s) is overflow\n\n", optarg);
                ShowHelpInfo(argv[0]);
                return -1;
            }
            else if (procs <= 0) {
                fprintf(stderr, "The interval of statistics must be > 0\n\n");
                ShowHelpInfo(argv[0]);
                return -1;
            }
            break;
        case '?':
            fprintf (stderr, "Unknown option -%c\n\n", optopt);
            ShowHelpInfo(argv[0]);
            return -1;
        case ':':
           fprintf (stderr, "Option -%c requires an argument\n\n", optopt);
           ShowHelpInfo(argv[0]);
           return -1;
        default:
            abort();  
        }
    }
    fprintf(stderr, "processes:  %ld\n", procs);
    fprintf(stderr, "duration:   %lds\n", duration);
    fprintf(stderr, "interval:   %lds\n", interval);
    memset(opt, 0, sizeof(Options));
    opt->procs = procs;
    opt->duration = duration;
    opt->interval = interval;

    return 0;
}


