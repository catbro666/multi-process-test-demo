#ifndef HEADER_WORK_H
#define HEADER_WORK_H
#include <openssl/evp.h>
#include <openssl/bio.h>
#include "opt.h"

/* 自定义测试参数 */
typedef struct hash_param_st {
    long index;
    unsigned char *data;
    unsigned int data_len;
    unsigned char md[128];
    unsigned int md_len;
    EVP_MD_CTX *ctx;
} HashParam;

typedef struct sign_param_st {
    long index;
    unsigned char data[48];
    size_t data_len;
    unsigned char sig[256];
    size_t sig_len;
    EVP_PKEY_CTX *ctx;
    BIO *in;
} SignParam;

typedef SignParam VerifyParam;

typedef struct enc_param_st {
    long index;
    unsigned char data[48];
    size_t data_len;
    unsigned char enc[256];
    size_t enc_len;
    EVP_PKEY_CTX *ctx;
    BIO *in;
} EncParam;

typedef EncParam DecParam;

/* 自定义测试结果 */
typedef struct result_st {
    unsigned long long count;
} Result;

typedef void (*init_fn) (Options *opt, void **param, long proc_index);
typedef void (*work_fn) (void *param);
typedef void (*clean_fn) (void *param);

extern init_fn test_init;
extern work_fn test_work;
extern clean_fn test_clean;

void global_init(Options *opt);

void global_clean();

#endif /* HEADER_WORK_H */
