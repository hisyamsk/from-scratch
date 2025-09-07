#include "hash_table.h"
#include "allocator.h"
#include "test.h"
#include "utils.h"
#include <stddef.h>
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
