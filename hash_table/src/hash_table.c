#include "hash_table.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    unsigned int hash;
    void *key;
    size_t key_len;
    void *val;
} ht_entry_t;

typedef struct {
    ht_entry_t *items;
    size_t len;
    size_t capacity;
} ht_bucket_t;

struct ht {
    ht_bucket_t *buckets;
    size_t capacity;
    size_t size;
    ht_config_t config;
};

static int bucket_reserve(ht_bucket_t *b, size_t new_cap) {
    if (new_cap <= b->capacity)
        return HT_OK;

    size_t capacity = (b->capacity == 0 ? 4 : b->capacity * 2);
    while (capacity < new_cap)
        capacity *= 2;

    ht_entry_t *new_items = realloc(b->items, capacity * sizeof(ht_entry_t));
    if (!new_items)
        return HT_ENONEM;

    b->items = new_items;
    b->capacity = capacity;
    return HT_OK;
}

static int bucket_find(const ht_bucket_t *b, unsigned hash, const void *key, size_t key_len,
                       const ht_config_t *config) {
    for (size_t i = 0; i < b->len; i++) {
        if (b->items[i].hash == hash &&
            config->equals(b->items[i].key, b->items[i].key_len, key, key_len)) {
            return (int) i;
        }
    }
    return -1;
}

static int bucket_insert(ht_bucket_t *b, unsigned hash, void *key, size_t key_len, void *val,
                         const ht_config_t *config) {
    int idx = bucket_find(b, hash, key, key_len, config);
    if (idx >= 0) {
        if (config->free_val)
            config->free_val(b->items[idx].val);
        b->items[idx].val = val;
        return HT_OK;
    }

    int err = bucket_reserve(b, b->len + 1);
    if (err != HT_OK)
        return err;

    b->items[idx].hash = hash;
    b->items[idx].key = key;
    b->items[idx].key_len = key_len;
    b->items[idx].val = val;
    b->len++;
    return HT_OK;
}
