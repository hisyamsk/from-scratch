#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stddef.h>

typedef enum {
    HT_OK = 0,
    HT_ERR = -1,
    HT_ENONEM = -2,
    HT_ENOTFOUND = -3,
    HT_EEXISTS = -4
} ht_err_t;

typedef struct ht ht_t;

typedef struct {
    unsigned int (*hash)(const void *key, size_t len, unsigned int seed);
    int (*equals)(const void *a, size_t alen, const void *b, size_t blen);

    void *(*dup_val)(const void *val);
    void *(*free_val)(void *val);

    void *(*dup_key)(const void *val);
    void *(*free_key)(void *key);

    unsigned int seed;
    double load_factor;
    size_t initial_capacity;
} ht_config_t;

typedef struct {
    size_t bucket;
    size_t index;
} ht_iter_t;

ht_t *ht_create(const ht_config_t *cfg);

void ht_destroy(ht_t *ht);

ht_err_t ht_set(ht_t *ht, const void *key, size_t key_len, const void *val);

ht_err_t ht_get(ht_t *ht, const void *key, size_t key_len, void **out_val);

ht_err_t ht_delete(ht_t *ht, const void *key, size_t key_len);

int ht_has(ht_t *ht, const void *key, size_t key_len);

size_t ht_size(const ht_t *ht);

ht_err_t ht_reserve(ht_t *ht, size_t n);

void ht_clear(ht_t *ht);

ht_iter_t ht_iter_begin(void);
int ht_iter_next(ht_t *ht, ht_iter_t *it, const void **key, size_t *key_len, void **val);

#endif
