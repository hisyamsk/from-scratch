#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>

void *alloc_mem(size_t size);
void *realloc_mem(void *ptr, size_t size);
void *calloc_mem(size_t nmemb, size_t size);
void free_mem(void *ptr);

#endif
