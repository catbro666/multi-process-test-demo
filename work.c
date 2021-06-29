#include "work.h"
#include <string.h>
#include <openssl/engine.h>
#include <openssl/pem.h>
#include "common.h"

const char* so_path = "/usr/local/ssl/lib/myengine.so";

init_fn test_init = NULL;
work_fn test_work = NULL;
clean_fn test_clean = NULL;

static void hash_init(Options *opt, void **param, long proc_index);
static void hash_work(void *param);
static void hash_clean(void *param);
static void sign_init(Options *opt, void **param, long proc_index);
static void sign_work(void *param);
static void sign_clean(void *param);
static void verify_init(Options *opt, void **param, long proc_index);
static void verify_work(void *param);
static void verify_clean(void *param);
static void encrypt_init(Options *opt, void **param, long proc_index);
static void encrypt_work(void *param);
static void encrypt_clean(void *param);
static void decrypt_init(Options *opt, void **param, long proc_index);
static void decrypt_work(void *param);
static void decrypt_clean(void *param);


void global_init(Options *opt) {
    ENGINE *e = NULL;
    if (opt->test == hash) {
        test_init = hash_init;
        test_work = hash_work;
        test_clean = hash_clean;
    }
    else if (opt->test == sign) {
        test_init = sign_init;
        test_work = sign_work;
        test_clean = sign_clean;
    }
    else if (opt->test == verify) {
        test_init = verify_init;
        test_work = verify_work;
        test_clean = verify_clean;
    }
    else if (opt->test == enc) {
        test_init = encrypt_init;
        test_work = encrypt_work;
        test_clean = encrypt_clean;
    }
    else if (opt->test == dec) {
        test_init = decrypt_init;
        test_work = decrypt_work;
        test_clean = decrypt_clean;
    }
    OpenSSL_add_all_algorithms();
    /* ENGINE INIT */
    ENGINE_load_dynamic();
    if (!(e = ENGINE_by_id("dynamic"))) {
        fail("ENGINE_by_id(\"dynamic\") fail\n");
    }
    if (!ENGINE_ctrl_cmd_string(e, "SO_PATH", so_path, 0)) {
        fail("ENGINE_ctrl_cmd_string(\"SO_PATH\") fail, so_path = %s\n", 
             so_path);
    }
    if (!ENGINE_ctrl_cmd_string(e, "LIST_ADD", "1", 0)) {
        fail("ENGINE_ctrl_cmd_string(\"LIST_ADD\") fail\n");
    }
    if (!ENGINE_ctrl_cmd_string(e, "LOAD", NULL, 0)) {
        fail("ENGINE_ctrl_cmd_string(\"LOAD\") fail\n");
    }
    if (!ENGINE_init(e)) {
        fail("ENGINE_init() fail\n");
    }
    if (!ENGINE_ctrl_cmd_string( e, "ENGINE_SET_LOGLEVEL", opt->loglevel, 0)) {
        fail("ENGINE_ctrl_cmd_string(\"ENGINE_SET_LOGLEVEL\") fail\n");
    }
    if (!ENGINE_set_default(e, ENGINE_METHOD_ALL)) {
        fail("ENGINE_set_default() fail\n");
    }
    ENGINE_free(e);
    /* ENGINE INIT */
}

void global_clean() {
    EVP_cleanup();
}

static void hash_init(Options *opt, void **param, long proc_index) {
    HashParam *p;
    p = OPENSSL_malloc(sizeof(HashParam));
    fail_if(!p, "OPENSSL_malloc() fail\n");
    memset(p, 0, sizeof(HashParam));

    p->ctx = EVP_MD_CTX_create();
    fail_clean_if(!p->ctx, hash_clean, (void *)p, "EVP_MD_CTX_create() fail\n");
    EVP_MD_CTX_init(p->ctx);

    p->data = OPENSSL_malloc(opt->len);
    fail_clean_if(!p->data, hash_clean, (void *)p, "OPENSSL_malloc() fail\n");
    fail_clean_if(RAND_bytes(p->data, sizeof(opt->len)) <= 0, hash_clean, (void *)p,
                  "RAND_bytes() fail\n");
    p->data_len = opt->len;

    p->index = proc_index;

    *param = p;
}

static void hash_work(void *param) {
    HashParam *p = (HashParam *)param;
    const EVP_MD *md = EVP_get_digestbyname("SHASH");
    fail_clean_if(!md, hash_clean, param, 
                  "EVP_get_digestbyname(\"SHASH\") fail\n");
    fail_clean_if(!EVP_DigestInit_ex(p->ctx, md, NULL), 
                  hash_clean, param, "EVP_DigestInit_ex() fail\n");
    fail_clean_if(!EVP_DigestUpdate(p->ctx, p->data, p->data_len),
                  hash_clean, param, "EVP_DigestUpdate() fail\n");
    fail_clean_if(!EVP_DigestFinal_ex(p->ctx, p->md, &p->md_len),
                  hash_clean, param, "EVP_DigestFinal_ex() fail\n");
}

static void hash_clean(void *param) {
    HashParam *p = (HashParam *)param;
    if (p->data)
        OPENSSL_free(p->data);
    if (p->ctx)
        EVP_MD_CTX_destroy(p->ctx);
    OPENSSL_free(p);
}

static void sign_init(Options *opt, void **param, long proc_index) {
    SignParam *p;
    EVP_PKEY *pkey;
    p = OPENSSL_malloc(sizeof(SignParam));
    fail_if(!p, "OPENSSL_malloc() fail\n");
    memset(p, 0, sizeof(SignParam));

    p->in = BIO_new_file(opt->key, "r");
    fail_clean_if(!p->in, sign_clean, (void *)p, 
                  "BIO_new_file(%s) fail\n", opt->key);
    pkey = PEM_read_bio_PrivateKey(p->in, NULL, NULL, NULL);
    fail_clean_if(!pkey, sign_clean, (void *)p, 
                  "PEM_read_bio_PrivateKey() fail\n");
    p->ctx = EVP_PKEY_CTX_new(pkey, NULL);
    if (!p->ctx) {
        EVP_PKEY_free(pkey);
        fail_clean_if(1, sign_clean, (void *)p, "EVP_PKEY_CTX_new() fail\n");
    }
    fail_clean_if(!EVP_PKEY_sign_init(p->ctx), sign_clean, (void *)p, 
                  "EVP_PKEY_sign_init() fail\n");

    fail_clean_if(RAND_bytes(p->data, sizeof(p->data)) <= 0, sign_clean, (void *)p,
                  "RAND_bytes() fail\n");
    p->data_len = sizeof(p->data);
    p->sig_len = sizeof(p->sig);

    p->index = proc_index;
    *param = p;
}

static void sign_work(void *param) {
    SignParam *p = (SignParam *)param;
    fail_clean_if(!EVP_PKEY_sign(p->ctx, p->sig, &p->sig_len, p->data, 
                                 p->data_len), sign_clean, param,
                  "EVP_PKEY_sign() fail\n");
}

static void sign_clean(void *param) {
    SignParam *p = (SignParam *)param;
    if (p->ctx)
        EVP_PKEY_CTX_free(p->ctx);
    if (p->in)
        BIO_free(p->in);
    OPENSSL_free(p);
}

static void verify_init(Options *opt, void **param, long proc_index) {
    VerifyParam *p;
    EVP_PKEY *pkey;
    X509 *x;
    unsigned char data[48];
    size_t data_len;
    unsigned char sig[256];
    size_t sig_len;
    /* 先签名一次，获取数据 */
    SignParam *sp;
    sign_init(opt, (void **)&sp, (long)0);
    sign_work(sp);
    memcpy(data, sp->data, sp->data_len);
    data_len = sp->data_len;
    memcpy(sig, sp->sig, sp->sig_len);
    sig_len = sp->sig_len;
    sign_clean(sp);

    p = OPENSSL_malloc(sizeof(VerifyParam));
    fail_if(!p, "OPENSSL_malloc() fail\n");
    memset(p, 0, sizeof(VerifyParam));
    
    p->in = BIO_new_file(opt->cert, "r");
    fail_clean_if(!p->in, verify_clean, (void *)p, 
                  "BIO_new_file(%s) fail\n", opt->cert);
    x = PEM_read_bio_X509(p->in, NULL, 0, NULL);
    fail_clean_if(!x, verify_clean, (void *)p, 
                  "PEM_read_bio_X509() fail\n");
    pkey = X509_get_pubkey(x);
    X509_free(x);
    fail_clean_if(!pkey, verify_clean, (void *)p, "X509_get_pubkey() fail\n");
    p->ctx = EVP_PKEY_CTX_new(pkey, NULL);
    if (!p->ctx) {
        EVP_PKEY_free(pkey);
        fail_clean_if(1, verify_clean, (void *)p, "EVP_PKEY_CTX_new() fail\n");
    }
    fail_clean_if(!EVP_PKEY_verify_init(p->ctx), verify_clean, (void *)p, 
                  "EVP_PKEY_verify_init() fail\n");

    memcpy(p->data, data, data_len);
    p->data_len = data_len;
    memcpy(p->sig, sig, sig_len);
    p->sig_len = sig_len;

    p->index = proc_index;
    *param = p;
}

static void verify_work(void *param) {
    VerifyParam *p = (VerifyParam *)param;
    fail_clean_if(!EVP_PKEY_verify(p->ctx, p->sig, p->sig_len, p->data, 
                                 p->data_len), verify_clean, param,
                  "EVP_PKEY_verify() fail\n");
}

static void verify_clean(void *param) {
    VerifyParam *p = (VerifyParam *)param;
    if (p->ctx)
        EVP_PKEY_CTX_free(p->ctx);
    if (p->in)
        BIO_free(p->in);
    OPENSSL_free(p);
}

static void encrypt_init(Options *opt, void **param, long proc_index) {
    EncParam *p;
    EVP_PKEY *pkey;
    X509 *x;
    p = OPENSSL_malloc(sizeof(EncParam));
    fail_if(!p, "OPENSSL_malloc() fail\n");
    memset(p, 0, sizeof(EncParam));

    p->in = BIO_new_file(opt->cert, "r");
    fail_clean_if(!p->in, encrypt_clean, (void *)p, 
                  "BIO_new_file(%s) fail\n", opt->cert);
    x = PEM_read_bio_X509(p->in, NULL, 0, NULL);
    fail_clean_if(!x, encrypt_clean, (void *)p, 
                  "PEM_read_bio_X509() fail\n");
    pkey = X509_get_pubkey(x);
    X509_free(x);
    fail_clean_if(!pkey, encrypt_clean, (void *)p, "X509_get_pubkey() fail\n");
    p->ctx = EVP_PKEY_CTX_new(pkey, NULL);
    if (!p->ctx) {
        EVP_PKEY_free(pkey);
        fail_clean_if(1, encrypt_clean, (void *)p, "EVP_PKEY_CTX_new() fail\n");
    }
    fail_clean_if(!EVP_PKEY_encrypt_init(p->ctx), encrypt_clean, (void *)p, 
                  "EVP_PKEY_encrypt_init() fail\n");

    fail_clean_if(RAND_bytes(p->data, sizeof(p->data)) <= 0, encrypt_clean, (void *)p,
                  "RAND_bytes() fail\n");
    p->data_len = sizeof(p->data);
    p->enc_len = sizeof(p->enc);

    p->index = proc_index;
    *param = p;
}

static void encrypt_work(void *param) {
    EncParam *p = (EncParam *)param;
    fail_clean_if(!EVP_PKEY_encrypt(p->ctx, p->enc, &p->enc_len, p->data, 
                                 p->data_len), encrypt_clean, param,
                  "EVP_PKEY_encrypt() fail\n");
}

static void encrypt_clean(void *param) {
    EncParam *p = (EncParam *)param;
    if (p->ctx)
        EVP_PKEY_CTX_free(p->ctx);
    if (p->in)
        BIO_free(p->in);
    OPENSSL_free(p);
}

static void decrypt_init(Options *opt, void **param, long proc_index) {
    DecParam *p;
    EVP_PKEY *pkey;
    unsigned char data[48];
    size_t data_len;
    unsigned char enc[256];
    size_t enc_len;
    /* 先加密一次，获取数据 */
    EncParam *ep;
    encrypt_init(opt, (void **)&ep, (long)0);
    encrypt_work(ep);
    memcpy(data, ep->data, ep->data_len);
    data_len = ep->data_len;
    memcpy(enc, ep->enc, ep->enc_len);
    enc_len = ep->enc_len;
    encrypt_clean(ep);

    p = OPENSSL_malloc(sizeof(DecParam));
    fail_if(!p, "OPENSSL_malloc() fail\n");
    memset(p, 0, sizeof(DecParam));
    
    p->in = BIO_new_file(opt->key, "r");
    fail_clean_if(!p->in, decrypt_clean, (void *)p, 
                  "BIO_new_file(%s) fail\n", opt->key);
    pkey = PEM_read_bio_PrivateKey(p->in, NULL, NULL, PEM_AUTO_KEYPASS);
    fail_clean_if(!pkey, decrypt_clean, (void *)p, 
                  "PEM_read_bio_PrivateKey() fail\n");
    p->ctx = EVP_PKEY_CTX_new(pkey, NULL);
    if (!p->ctx) {
        EVP_PKEY_free(pkey);
        fail_clean_if(1, decrypt_clean, (void *)p, "EVP_PKEY_CTX_new() fail\n");
    }
    fail_clean_if(!EVP_PKEY_decrypt_init(p->ctx), decrypt_clean, (void *)p, 
                  "EVP_PKEY_decrypt_init() fail\n");

    memcpy(p->data, data, data_len);
    p->data_len = data_len;
    memcpy(p->enc, enc, enc_len);
    p->enc_len = enc_len;

    p->index = proc_index;
    *param = p;
}

static void decrypt_work(void *param) {
    DecParam *p = (DecParam *)param;
    fail_clean_if(!EVP_PKEY_decrypt(p->ctx, p->data, &p->data_len, p->enc, 
                                 p->enc_len), decrypt_clean, param,
                  "EVP_PKEY_decrypt() fail\n");
}

static void decrypt_clean(void *param) {
    DecParam *p = (DecParam *)param;
    if (p->ctx)
        EVP_PKEY_CTX_free(p->ctx);
    if (p->in)
        BIO_free(p->in);
    OPENSSL_free(p);
}


