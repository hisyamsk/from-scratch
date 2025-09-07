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
