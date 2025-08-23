#define _DEFAULT_SOURCE
#include "allocator.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MIN_ALLOC 1024

typedef long Align;

union header {
    struct {
        union header *next;
        size_t size;
    } s;
    Align x;
};

typedef union header Header;

static Header base;
static Header *freep = NULL;
static char *heap_start = NULL;
static char *heap_end = NULL;

void free_mem(void *ptr) {
    if (!ptr)
        return;

    if ((char *) ptr <= heap_start || (char *) ptr >= heap_end) {
        fprintf(stderr, "free_mem: invalid pointer %p\n", ptr);
        return;
    }

    Header *ptr_header = (Header *) ptr - 1;
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

void *more_mem(size_t n_units) {
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
    free_mem((void *) (new_mem + 1));
    return new_mem;
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
            return (void *) (curr + 1);
        }

        if (prev == curr->s.next) {
            if (more_mem(n_units) == (void *) -1)
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
