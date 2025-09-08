#include "hash_table.h"
#include "allocator.h"
#include "test.h"
#include "utils.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static void *dup_mem(const void *src, size_t size) {
    if (!src || !size) return NULL;

    void *dest = alloc_mem(size);
    if (!dest) return NULL;

    memcpy(dest, src, size);
    return dest;
}

static int mem_eq(const void *a, size_t alen, const void *b, size_t blen) {
    if (!a || !b || alen != blen) return 0;
    return memcmp(a, b, alen) == 0;
}

ht_config_t default_config = {
    .hash = fnv1a64,
    .dup_key = dup_mem,
    .dup_val = dup_mem,
    .equals = mem_eq,
    .free_key = free_mem,
    .free_val = free_mem,
    .seed = 0xDEADABADCAFEC,
};

TEST(ht_create_destroy) {
    ht_t *ht = ht_create(&default_config);
    ASSERT_NOT_NULL("ht should not be null", ht);
    ASSERT_INT_EQUAL("ht's initial size should be 0", 0, (int) ht_size(ht));
    ht_destroy(ht);
}

TEST(ht_insert_lookup) {
    ht_t *ht = ht_create(&default_config);
    ASSERT_NOT_NULL("ht should not be null", ht);

    ht_err_t err = ht_set(ht, "name", 4, "hisyam", 6);
    ASSERT_INT_EQUAL("ht_set should not return error", HT_OK, err);

    void *val = NULL;
    err = ht_get(ht, "name", 4, &val);

    ASSERT_INT_EQUAL("ht_set should not return error", HT_OK, err);
    ASSERT_INT_EQUAL("ht_set should increment size", 1, (int) ht_size(ht));
    ASSERT_NOT_NULL("val should not be null", val);
    ASSERT_STR_EQUAL("value should match inserted", "hisyam", (char *) val, 6);

    err = ht_has(ht, "name", 4);

    ht_destroy(ht);
}

TEST(ht_update_value) {
    ht_t *ht = ht_create(&default_config);
    ASSERT_NOT_NULL("ht should not be null", ht);

    ht_err_t err = ht_set(ht, "name", 4, "hisyam", 6);
    err = ht_set(ht, "name", 4, "kurniawan", 9);
    ASSERT_INT_EQUAL("ht_set should not return error", HT_OK, err);
    ASSERT_INT_EQUAL("update should not increment size", 1, (int) ht_size(ht));

    void *val = NULL;
    err = ht_get(ht, "name", 4, &val);
    ASSERT_STR_EQUAL("value should be updated", "kurniawan", (char *) val, 9);

    ht_destroy(ht);
}

TEST(ht_has_and_delete) {
    ht_t *ht = ht_create(&default_config);
    ASSERT_NOT_NULL("ht should not be null", ht);

    ht_err_t err = ht_set(ht, "name", 4, "hisyam", 6);
    ASSERT_INT_EQUAL("ht_set should not return error", HT_OK, err);

    err = ht_delete(ht, "name", 4);
    ASSERT_INT_EQUAL("ht_delete should not return error", HT_OK, err);

    err = ht_has(ht, "name", 4);
    ASSERT_INT_EQUAL("value should be deleted", HT_ENOTFOUND, err);

    ht_destroy(ht);
}

TEST(ht_resize_trigger) {
    ht_t *ht = ht_create(&default_config);
    ASSERT_NOT_NULL("ht should not be null", ht);

    size_t old_cap = ht_capacity(ht);
    int insert_count = (int) (old_cap * 0.75) + 1;

    for (int i = 0; i < insert_count; i++) {
        char key[16];
        snprintf(key, sizeof(key), "k%d", i);
        ht_set(ht, key, strlen(key), "val", 3);
    }

    ASSERT_INT_EQUAL("capacity should grow", 32, (int) ht_capacity(ht));
    ASSERT_UINT_EQUAL("size should equal number of inserts", insert_count, (int) ht_size(ht));

    ht_destroy(ht);
}

TEST(ht_clear_zero_size) {
    ht_t *ht = ht_create(&default_config);
    ASSERT_NOT_NULL("ht should not be null", ht);

    ht_set(ht, "name", 4, "hisyam", 6);
    ht_set(ht, "country", 7, "indonesia", 9);
    ht_set(ht, "city", 4, "jakarta", 7);

    ASSERT_TRUE("size should be 3", 3 == ht_size(ht));

    ht_clear(ht);

    ASSERT_TRUE("size should be 0", 0 == ht_size(ht));
    ASSERT_INT_EQUAL("deleted key should not exists", HT_ENOTFOUND, ht_has(ht, "name", 4));

    ht_destroy(ht);
}

TEST(ht_iterator) {
    ht_t *ht = ht_create(&default_config);
    ASSERT_NOT_NULL("ht should not be null", ht);
    int val1 = 1, val2 = 2, val3 = 3;

    ht_set(ht, "a", 1, (void *) &val1, sizeof(int));
    ht_set(ht, "b", 1, (void *) &val2, sizeof(int));
    ht_set(ht, "c", 1, (void *) &val3, sizeof(int));

    ASSERT_TRUE("size should be 3", 3 == ht_size(ht));

    ht_iter_t hi = ht_iter_begin(ht);
    void *key;
    size_t key_len;
    void *val;
    int count = 0;

    while (ht_iter_next(&hi, &key, &key_len, &val)) {
        count++;
        ASSERT_NOT_NULL("key should not be null", key);
        ASSERT_NOT_NULL("val should not be null", val);
    }

    ASSERT_INT_EQUAL("iterator should visit all elements", 3, count);

    ht_destroy(ht);
}
