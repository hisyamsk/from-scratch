#include "utils.h"

uint32_t fnv1a32(const void *data, size_t len, uint64_t seed) {
    const unsigned char *p = (const unsigned char *) data;
    uint32_t hash = 2166136261u ^ seed;
    for (size_t i = 0; i < len; i++) {
        hash ^= (uint32_t) p[i];
        hash *= 16777619u;
    }
    return hash;
}

uint64_t fnv1a64(const void *data, size_t len, uint64_t seed) {
    const unsigned char *p = (const unsigned char *) data;
    uint64_t hash = 14695981039346656037ULL ^ seed;
    for (size_t i = 0; i < len; i++) {
        hash ^= (uint64_t) p[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}
