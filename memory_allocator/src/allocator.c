#include "allocator.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MIN_ALLOC 1024
#define MAGIC_ALLOCATED 0xDEADBEEF
#define MAGIC_FREED 0xABADCAFE
#define POISON_BYTE 0xAA

typedef long Align;

union header {
    struct {
        union header *next;
        size_t size;
        uint32_t magic;
    } s;
    Align x;
};

typedef union header Header;

static Header base;
static Header *freep = NULL;
static char *heap_start = NULL;
static char *heap_end = NULL;

static int validate_ptr(void *ptr, const char *funcname) {
    if (!ptr)
        return 0;

    if ((char *) ptr <= (char *) heap_start || (char *) ptr >= (char *) heap_end) {
        fprintf(stderr, "%s: invalid pointer %p (out of heap bounds)\n", funcname, ptr);
        return 0;
    }

    Header *ptr_header = (Header *) ptr - 1;

    if (ptr_header->s.magic == MAGIC_FREED) {
        fprintf(stderr, "%s: double free detected at %p\n", funcname, ptr);
        return 0;
    }
    if (ptr_header->s.magic != MAGIC_ALLOCATED) {
        fprintf(stderr, "%s: invalid pointer %p (heap corruption)\n", funcname, ptr);
        return 0;
    }

    return 1;
}

void free_mem(void *ptr) {
    if (!ptr)
        return;
    if (!validate_ptr(ptr, "free_mem"))
        return;

    Header *ptr_header = (Header *) ptr - 1;
    size_t user_data = (ptr_header->s.size - 1) * sizeof(Header);
    memset(ptr, POISON_BYTE, user_data);

    ptr_header->s.magic = MAGIC_FREED;

    Header *curr = freep;
    for (; !(ptr_header > curr && ptr_header < curr->s.next); curr = curr->s.next) {
        if (curr >= curr->s.next && (ptr_header > curr || ptr_header < curr->s.next))
            break;
    }

    if (ptr_header + ptr_header->s.size == curr->s.next) {
        ptr_header->s.size += curr->s.next->s.size;
        ptr_header->s.next = curr->s.next->s.next;
    } else {
        ptr_header->s.next = curr->s.next;
    }

    if (curr + curr->s.size == ptr_header) {
        curr->s.size += ptr_header->s.size;
        curr->s.next = ptr_header->s.next;
    } else {
        curr->s.next = ptr_header;
    }

    freep = curr;
}

static void *more_mem(size_t n_units) {
    if (heap_start == NULL)
        heap_start = (char *) sbrk(0);
    if (n_units < MIN_ALLOC)
        n_units = MIN_ALLOC;

    void *p = sbrk(n_units * sizeof(Header));
    if (p == (void *) -1)
        return NULL;

    heap_end = (char *) sbrk(0);

    Header *new_mem = (Header *) p;
    new_mem->s.size = n_units;
    new_mem->s.magic = MAGIC_ALLOCATED;
    free_mem((void *) (new_mem + 1));
    return freep;
}

void *alloc_mem(size_t n_bytes) {
    size_t n_units = (n_bytes + sizeof(Header) - 1) / sizeof(Header) + 1;

    if (!freep) {
        base.s.next = freep = &base;
        base.s.size = 0;
    }

    Header *prev = freep;
    for (Header *curr = prev->s.next;; prev = curr, curr = curr->s.next) {
        if (curr->s.size >= n_units) {
            if (curr->s.size == n_units) {
                prev->s.next = curr->s.next;
            } else {
                curr->s.size -= n_units;
                curr += curr->s.size;
                curr->s.size = n_units;
            }

            freep = prev;
            curr->s.magic = MAGIC_ALLOCATED;
            return (void *) (curr + 1);
        }

        if (curr == freep) {
            if (more_mem(n_units) == NULL)
                return NULL;
        }
    }
}

void *calloc_mem(size_t nmemb, size_t size) {
    if (nmemb != 0 && size > SIZE_MAX / nmemb)
        return NULL;

    size_t total = nmemb * size;
    void *mem = alloc_mem(total);
    if (mem != NULL)
        memset(mem, 0, total);

    return mem;
}

void *realloc_mem(void *ptr, size_t size) {
    if (!ptr)
        return NULL;
    if (size == 0) {
        free_mem(ptr);
        return NULL;
    }
    if (!validate_ptr(ptr, "realloc_mem"))
        return NULL;

    void *new_mem = alloc_mem(size);
    memcpy(new_mem, ptr, size);
    free_mem(ptr);

    return new_mem;
}
