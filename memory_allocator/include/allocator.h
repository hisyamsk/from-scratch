#ifndef MANAGER_H
#define MANAGER_H

#include <stddef.h>

void *alloc_mem(size_t size);
void *realloc_mem(void *ptr, size_t size);
void *calloc_mem(size_t nmemb, size_t size);
void free_mem(void *ptr);

#endif
