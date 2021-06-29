#include "opt.h"    

#include <limits.h>     // LONG_MIN, LONG_MAX, ULLONG_MAX
#include <string.h>     // memset
#include <stdlib.h>     // strtol, abort
#include <getopt.h>     // geropt_long

#include "common.h"     // mylog

static void ShowHelpInfo(char *name) {
    mylog("Usage: %s [options]\n\n", name);
    mylog("  Options:\n");
    mylog("    -p/--proc         Number of processes (default: 1)\n");
    mylog("    -d/--duration     Duration of test (unit: s, default: 10)\n");
    mylog("    -i/--interval     Interval of statisics (unit: s, default: 1)\n");
    mylog("    -t/--test         Test case (hash|sign|verify|enc|dec, default: hash)\n");
    mylog("    -l/--len          Hash data len (unit: byte, default: 1024)\n");
    mylog("    -k/--key          PEM Key file path (default: ./key.pem)\n");
    mylog("    -c/--cert         PEM Cert file path (default: ./cert.pem)\n");
    mylog("    -o/--loglevel     Engine log level (0-9, default: 0)\n");
    mylog("    -h/--help         Show the help info\n");
    mylog("\n");
    mylog("  Example:\n");
    mylog("    %s -p 1 -d 30 -i 1 -t sign -k key.pem -c cert.pem\n", name);
    mylog("\n");
}

/* 处理参数 */
int process_options(int argc, char *argv[], Options *opt) {
    int c = 0;
    int option_index = 0;
    long procs = 1;
    long duration = 10;
    long interval = 1;
    Test test = hash;
    long len = 1024;
    const char *key = "./key.pem";
    const char *cert = "./cert.pem";
    const char *loglevel = "0";
    /**     
     *  定义命令行参数列表，option结构的含义如下（详见 man 3 getopt）：
     *  struct option {
     *      const char *name;       // 参数的完整名称，对应命令中的 --xxx
     *      int  has_arg;           // 该参数是否带有一个值，如 –config xxx.conf
     *      int *flag;              // 一般设置为NULL
     *      int  val;               // 解析到该参数后getopt_long函数的返回值，
     *                      // 为了方便维护，一般对应getopt_long调用时第三个参数
     *  };
     */
    static struct option arg_options[] = 
    {
        {"proc", 1, NULL, 'p'},
        {"duration", 1, NULL, 'd'},
        {"interval", 1, NULL, 'i'},
        {"test", 1, NULL, 't'},
        {"len", 1, NULL, 'l'},
        {"key", 1, NULL, 'k'},
        {"cert", 1, NULL, 'c'},
        {"log", 1, NULL, 'g'},
        {"help", 0, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };

    /**
     *  注意：传递给getopt_long的第三个参数对应了命令行参数的缩写形式，如-h等，
     *  如果字符后面带有冒号":"，则说明该参数后跟一个值，如-c xxxxxx             
     *  如果开头有冒号":"，则当一个选项缺少参数时，返回":"，否则，返回"?"
     */
    while ((c = getopt_long(argc, argv, ":p:d:i:t:l:k:c:g:h", arg_options, &option_index)
            ) != -1) {
        switch (c) {
        case 'h':
            ShowHelpInfo(argv[0]);
            //fprintf(stderr,"option is -%c, optarv is %s\n", c, optarg);
            exit(0);
        case 'p':
            procs = strtol(optarg, NULL, 0);
            if (procs == LONG_MIN || procs == LONG_MAX) {
                mylog("The number of processes (%s) is overflow\n\n", optarg);
                ShowHelpInfo(argv[0]);
                return -1;
            }
            else if (procs <= 0) {
                mylog("The number of processes must be > 0\n\n");
                ShowHelpInfo(argv[0]);
                return -1;
            }
            break;
        case 'd':
            duration = strtol(optarg, NULL, 0);
            if (duration == LONG_MIN || duration == LONG_MAX) {
                mylog("The duration of test (%s) is overflow\n\n", optarg);
                ShowHelpInfo(argv[0]);
                return -1;
            }
            else if (duration <= 0) {
                mylog("The duration of test must be > 0\n\n");
                ShowHelpInfo(argv[0]);
                return -1;
            }
            break;
        case 'i':
            interval = strtol(optarg, NULL, 0);
            if (interval == LONG_MIN || interval == LONG_MAX) {
                mylog("The interval of statistics (%s) is overflow\n\n", 
                      optarg);
                ShowHelpInfo(argv[0]);
                return -1;
            }
            else if (interval <= 0) {
                mylog("The interval of statistics must be > 0\n\n");
                ShowHelpInfo(argv[0]);
                return -1;
            }
            break;
        case 't':
            if(!strcasecmp("hash", optarg)) {
                test = hash;
            }
            else if(!strcasecmp("sign", optarg)) {
                test = sign;
            }
            else if(!strcasecmp("verify", optarg)) {
                test = verify;
            }
            else if(!strcasecmp("enc", optarg)) {
                test = enc;
            }
            else if(!strcasecmp("dec", optarg)) {
                test = dec;
            }
            else {
                mylog("Unknown test case type\n\n");
                ShowHelpInfo(argv[0]);
                return -1;
            }
            break;
        case 'l':
            len = strtol(optarg, NULL, 0);
            if (len == LONG_MIN || len == LONG_MAX) {
                mylog("The len of hash data (%s) is overflow\n\n", 
                      optarg);
                ShowHelpInfo(argv[0]);
                return -1;
            }
            else if (len <= 0) {
                mylog("The len of hash data must be > 0\n\n");
                ShowHelpInfo(argv[0]);
                return -1;
            }
            break;
        case 'k':
            key = optarg;
            break;
        case 'c':
            cert = optarg;
            break;
        case 'g':
            loglevel = optarg;
            break;
        case '?':
            mylog("Unknown option -%c\n\n", optopt);
            ShowHelpInfo(argv[0]);
            return -1;
        case ':':
            mylog("Option -%c requires an argument\n\n", optopt);
            ShowHelpInfo(argv[0]);
            return -1;
        default:
            exit(1);  
        }
    }
    mylog("processes:  %ld\n", procs);
    mylog("duration:   %lds\n", duration);
    mylog("interval:   %lds\n", interval);
    mylog("test:       #%d\n", test);
    mylog("len:        %ld bytes\n", len);
    mylog("key:        %s\n", key);
    mylog("cert:       %s\n", cert);
    mylog("loglevel:   %s\n", loglevel);
    memset(opt, 0, sizeof(Options));
    opt->procs = procs;
    opt->duration = duration;
    opt->interval = interval;
    opt->test = test;
    opt->len = len;
    opt->key = key;
    opt->cert = cert;
    opt->loglevel = loglevel;

    return 0;
}
