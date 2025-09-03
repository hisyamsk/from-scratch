#include "hash_table.h"
#include "allocator.h"
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

static int bucket_reserve(ht_bucket_t *bucket, size_t new_capacity) {
    if (new_capacity <= bucket->capacity)
        return HT_OK;

    size_t capacity = (bucket->capacity == 0 ? 4 : bucket->capacity * 2);
    while (capacity < new_capacity)
        capacity *= 2;

    ht_entry_t *new_items = realloc_mem(bucket->items, capacity * sizeof(ht_entry_t));
    if (!new_items)
        return HT_ENONEM;

    bucket->items = new_items;
    bucket->capacity = capacity;
    return HT_OK;
}

static int bucket_find(const ht_bucket_t *bucket, unsigned hash, const void *key, size_t key_len,
                       const ht_config_t *config) {
    for (size_t i = 0; i < bucket->len; i++) {
        if (bucket->items[i].hash == hash &&
            config->equals(bucket->items[i].key, bucket->items[i].key_len, key, key_len)) {
            return (int) i;
        }
    }
    return -1;
}

static int bucket_insert(ht_bucket_t *bucket, unsigned hash, void *key, size_t key_len, void *val,
                         const ht_config_t *config) {
    int idx = bucket_find(bucket, hash, key, key_len, config);
    if (idx >= 0) {
        if (config->free_val)
            config->free_val(bucket->items[idx].val);
        bucket->items[idx].val = val;
        return HT_OK;
    }

    int err = bucket_reserve(bucket, bucket->len + 1);
    if (err != HT_OK)
        return err;

    bucket->items[idx].hash = hash;
    bucket->items[idx].key = key;
    bucket->items[idx].key_len = key_len;
    bucket->items[idx].val = val;
    bucket->len++;
    return HT_OK;
}

static int bucket_delete(ht_bucket_t *bucket, unsigned hash, void *key, size_t key_len,
                         const ht_config_t *config) {
    int idx = bucket_find(bucket, hash, key, key_len, config);
    if (idx < 0)
        return HT_ENOTFOUND;

    if (config->free_key)
        config->free_val(bucket->items[idx].key);
    if (config->free_val)
        config->free_val(bucket->items[idx].val);

    bucket->items[idx] = bucket->items[bucket->len - 1];
    bucket->len--;
    return HT_OK;
}

#define DEFAULT_INITIAL_CAPACITY 16
#define DEFAULT_LOAD_FACTOR 0.75

static size_t next_pow2(size_t n) {
    size_t p = 1;
    while (p < n)
        p <<= 1;
    return p;
}

ht_t *ht_create(const ht_config_t *config) {
    if (!config)
        return NULL;

    ht_t *ht = calloc_mem(1, sizeof(ht_t));
    if (!ht)
        return NULL;

    ht->config = *config;
    if (ht->config.initial_capacity == 0)
        ht->config.initial_capacity = DEFAULT_INITIAL_CAPACITY;

    if (ht->config.load_factor <= 0.0)
        ht->config.load_factor = DEFAULT_LOAD_FACTOR;

    ht->capacity = next_pow2(ht->config.initial_capacity);
    ht->size = 0;

    ht->buckets = calloc_mem(ht->capacity, sizeof(ht_bucket_t));
    if (ht->buckets) {
        free(ht);
        return NULL;
    }

    return ht;
}

void ht_destroy(ht_t *ht) {
    if (!ht)
        return;

    for (size_t i = 0; i < ht->capacity; i++) {
        ht_bucket_t *bucket = &ht->buckets[i];
        for (size_t j = 0; j < bucket->capacity; j++) {
            ht_entry_t *item = &bucket->items[j];
            if (ht->config.free_key && item->key)
                ht->config.free_key(item->key);
            if (ht->config.free_val && item->val)
                ht->config.free_val(item->val);
        }
        free_mem(bucket->items);
    }

    free_mem(ht->buckets);
    free_mem(ht);
}
