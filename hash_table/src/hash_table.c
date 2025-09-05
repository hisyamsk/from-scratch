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
    ht_entry_t *entries;
    size_t size;
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

    ht_entry_t *new_entries = realloc_mem(bucket->entries, capacity * sizeof(ht_entry_t));
    if (!new_entries)
        return HT_ENONEM;

    bucket->entries = new_entries;
    bucket->capacity = capacity;
    return HT_OK;
}

static int bucket_find(const ht_bucket_t *bucket, unsigned hash, const void *key, size_t key_len,
                       const ht_config_t *config) {
    for (size_t i = 0; i < bucket->size; i++) {
        if (bucket->entries[i].hash == hash &&
            config->equals(bucket->entries[i].key, bucket->entries[i].key_len, key, key_len)) {
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
            config->free_val(bucket->entries[idx].val);
        bucket->entries[idx].val = val;
        return HT_OK;
    }

    int err = bucket_reserve(bucket, bucket->size + 1);
    if (err != HT_OK)
        return err;

    bucket->entries[bucket->size].hash = hash;
    bucket->entries[bucket->size].key = key;
    bucket->entries[bucket->size].key_len = key_len;
    bucket->entries[bucket->size].val = val;
    bucket->size++;
    return HT_OK;
}

static int bucket_delete(ht_bucket_t *bucket, unsigned hash, void *key, size_t key_len,
                         const ht_config_t *config) {
    int idx = bucket_find(bucket, hash, key, key_len, config);
    if (idx < 0)
        return HT_ENOTFOUND;

    if (config->free_key)
        config->free_val(bucket->entries[idx].key);
    if (config->free_val)
        config->free_val(bucket->entries[idx].val);

    bucket->entries[idx] = bucket->entries[bucket->size - 1];
    bucket->size--;
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

static ht_err_t ht_resize(ht_t *ht, size_t new_capacity) {
    ht_bucket_t *new_buckets = calloc_mem(new_capacity, sizeof(ht_bucket_t));
    if (!new_buckets)
        return HT_ENONEM;

    for (size_t i = 0; i < ht->capacity; i++) {
        ht_bucket_t *old_bucket = &ht->buckets[i];
        for (size_t j = 0; j < old_bucket->size; j++) {
            ht_entry_t *entry = &old_bucket->entries[j];

            size_t idx = entry->hash & (new_capacity - 1);
            ht_bucket_t *new_bucket = &new_buckets[idx];

            int err = bucket_reserve(new_bucket, new_bucket->size + 1);
            if (err != HT_OK) {
                for (size_t k = 0; k < new_capacity; k++)
                    free_mem(new_buckets[k].entries);
                free_mem(new_buckets);
                return err;
            }

            new_bucket->entries[new_bucket->size++] = *entry;
        }
        free_mem(old_bucket->entries);
    }

    free_mem(ht->buckets);
    ht->buckets = new_buckets;
    ht->capacity = new_capacity;

    return HT_OK;
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
    if (!ht->buckets) {
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
            ht_entry_t *entry = &bucket->entries[j];
            if (ht->config.free_key && entry->key)
                ht->config.free_key(entry->key);
            if (ht->config.free_val && entry->val)
                ht->config.free_val(entry->val);
        }
        free_mem(bucket->entries);
    }

    free_mem(ht->buckets);
    free_mem(ht);
}

ht_err_t ht_set(ht_t *ht, const void *key, size_t key_len, void *val) {
    if (!ht || !key)
        return HT_ERR;

    if (ht->size + 1 > (size_t) (ht->capacity * ht->config.load_factor)) {
        ht_err_t err = ht_resize(ht, ht->capacity * 2);
        if (err != HT_OK)
            return err;
    }

    unsigned int hash = ht->config.hash(key, key_len, ht->config.seed);
    size_t idx = hash & (ht->capacity - 1);
    ht_bucket_t *bucket = &ht->buckets[idx];

    void *dup_key = ht->config.dup_key ? ht->config.dup_key(key, key_len) : (void *) key;
    void *dup_val = ht->config.dup_val ? ht->config.dup_val(val) : (void *) val;

    size_t prev_bucket_size = bucket->size;

    int err = bucket_insert(bucket, hash, dup_key, key_len, dup_val, &ht->config);
    if (err == HT_OK && bucket->size > prev_bucket_size)
        ht->size++;

    return err;
}

ht_err_t ht_get(ht_t *ht, const void *key, size_t key_len, void **out_val) {
    if (!ht || !key || !out_val)
        return HT_ERR;

    unsigned int hash = ht->config.hash(key, key_len, ht->config.seed);
    size_t idx = hash & (ht->capacity - 1);
    ht_bucket_t *bucket = &ht->buckets[idx];

    size_t i = bucket_find(bucket, hash, key, key_len, &ht->config);
    if (i < 0)
        return HT_ENOTFOUND;

    *out_val = bucket->entries[i].val;
    return HT_OK;
}

ht_err_t ht_delete(ht_t *ht, const void *key, size_t key_len) {
    if (!ht || !key)
        return HT_ERR;

    unsigned int hash = ht->config.hash(key, key_len, ht->config.seed);
    size_t idx = hash & (ht->capacity - 1);
    ht_bucket_t *bucket = &ht->buckets[idx];

    void *dup_key = ht->config.dup_key ? ht->config.dup_key(key, key_len) : (void *) key;

    int err = bucket_delete(bucket, hash, dup_key, key_len, &ht->config);
    if (err != HT_OK)
        return err;

    ht->size--;
    return err;
}

ht_err_t ht_has(ht_t *ht, const void *key, size_t key_len) {
    if (!ht || !key)
        return HT_ERR;

    unsigned int hash = ht->config.hash(key, key_len, ht->config.seed);
    size_t idx = hash & (ht->capacity - 1);
    ht_bucket_t *bucket = &ht->buckets[idx];

    return bucket_find(bucket, hash, key, key_len, &ht->config);
}

void ht_clear(ht_t *ht) {
    if (!ht)
        return;

    for (size_t i = 0; i < ht->capacity; i++) {
        ht_bucket_t *bucket = &ht->buckets[i];
        for (size_t j = 0; j < bucket->size; j++) {
            ht_entry_t *entry = &bucket->entries[j];
            int err = bucket_delete(bucket, entry->hash, entry->key, entry->key_len, &ht->config);
            if (err != HT_OK)
                return;
        }

        free_mem(bucket->entries);
        bucket->entries = NULL;
        bucket->size = 0;
        bucket->capacity = 0;
    }

    ht->size = 0;
}
