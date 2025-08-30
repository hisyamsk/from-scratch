#include "allocator.h"
#include "test.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

TEST(alloc_mem_returns_non_null) {
    int *arr = alloc_mem(10 * sizeof(int));
    ASSERT_NOT_NULL("arr should not be null", arr);
}

TEST(alloc_mem_stores_values_correctly) {
    int *arr = alloc_mem(10 * sizeof(int));
    ASSERT_NOT_NULL("arr should not be null", arr);

    int expected[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    for (int i = 0; i < 10; i++) {
        arr[i] = i + 1;
    }

    ASSERT_MEM_EQUAL("memory should be equal", expected, arr, 10 * sizeof(int));
}

TEST(alloc_mem_zero_size) {
    void *ptr = alloc_mem(0);
    ASSERT_NOT_NULL("alloc_mem(0) should be a valid pointer", ptr);
}

TEST(alloc_mem_multiple) {
    int *a = alloc_mem(10 * sizeof(int));
    int *b = alloc_mem(10 * sizeof(int));

    ASSERT_NOT_NULL("a should not be null", a);
    ASSERT_NOT_NULL("b should not be null", b);
    ASSERT_PTR_NOT_EQUAL("a and b should not be equal", a, b);
    ASSERT_TRUE("a and b should not overlap", (a + 10 <= b) || (b + 10 <= a));
}

TEST(alloc_mem_alignment) {
    void *ptr = alloc_mem(1);
    ASSERT_NOT_NULL("ptr should not be null", ptr);
    ASSERT_UINTPTR_EQUAL("ptr should be aligned", 0, (uintptr_t) ptr % sizeof(void *));
}

TEST(free_mem_success) {
    char *p = alloc_mem(32);
    ASSERT_NOT_NULL("p should not be null", p);

    int s, r;
    capture_stderr_start(&s, &r);

    free_mem(p);

    char *out = capture_stderr_end(s, r);
    ASSERT_TRUE("free_mem should not return any error", strlen(out) == 0);
    free(out);
}

TEST(free_stack_address_logs_error) {
    int s, r;
    capture_stderr_start(&s, &r);

    char a = 'a';
    free_mem(&a);

    char *out = capture_stderr_end(s, r);
    ASSERT_STR_MATCH("free_mem for stack address should log error", out, "out of heap bounds");
    free(out);
}

TEST(free_mem_offset_pointer_logs_error) {
    int s, r;
    capture_stderr_start(&s, &r);

    char *p = alloc_mem(32);
    ASSERT_NOT_NULL("p should not be null", p);

    free_mem(p + 3);

    char *out = capture_stderr_end(s, r);
    ASSERT_STR_MATCH("free_mem for offset pointer should log error", out, "heap corruption");
    free(out);
}

TEST(free_mem_unallocated_heap_logs_error) {
    int s, r;
    capture_stderr_start(&s, &r);

    char *p = alloc_mem(32);
    ASSERT_NOT_NULL("p should not be null", p);

    char *pp = (char *) sbrk(0);

    free_mem(pp - 10);

    char *out = capture_stderr_end(s, r);
    ASSERT_STR_MATCH("free_mem for unallocated heap address should log error", out,
                     "heap corruption");
    free(out);
}

TEST(double_free_logs_error) {
    char *p = alloc_mem(32);
    ASSERT_NOT_NULL("p should not be null", p);

    int s, r;
    capture_stderr_start(&s, &r);

    free_mem(p);
    free_mem(p);

    char *out = capture_stderr_end(s, r);
    ASSERT_STR_MATCH("double free_mem should log error", out, "double free");
    free(out);
}

TEST(use_after_free) {
    char *p = alloc_mem(10);
    ASSERT_NOT_NULL("p should not be null", p);
    strcpy(p, "ABCDEFGHIJ");

    free_mem(p);
    char expected[10];
    memset(expected, 0xAA, 10);

    ASSERT_MEM_EQUAL("memory should be filled with poison for UAF", expected, p, 10);
}

TEST(calloc_mem_success) {
    int *p = calloc_mem(10, sizeof(int));
    ASSERT_NOT_NULL("p should not be null", p);

    int expected[10] = {0};
    ASSERT_MEM_EQUAL("p values should be set to zero", expected, p, 10 * sizeof(int));
}

TEST(realloc_mem_success) {
    char *p1 = alloc_mem(11);
    ASSERT_NOT_NULL("p should not be null", p1);
    strcpy(p1, "ABCDEFGHIJ\0");

    char *p2 = realloc_mem(p1, 20);
    ASSERT_STR_EQUAL("p2 should have the same content from p1", "ABCDEFGHIJ\0", p2, 11);
}

TEST(realloc_zero_size) {
    char *p = alloc_mem(32);
    ASSERT_NOT_NULL("p should not be null", p);

    char *p2 = realloc_mem(p, 0);
    ASSERT_NULL("p2 should be null", p2);

    char expected[32];
    memset(expected, 0xAA, 32);

    ASSERT_MEM_EQUAL("realloc_mem should free and posion memory", expected, p, 32);
}

TEST(realloc_mem_stack_address_logs_error) {
    int s, r;
    capture_stderr_start(&s, &r);

    char *a = "hello\0";
    realloc_mem(a, 10);

    char *out = capture_stderr_end(s, r);
    ASSERT_STR_MATCH("realloc_mem for stack address should log error", out, "out of heap bounds");
    free(out);
}

TEST(realloc_mem_offset_pointer_logs_error) {
    int s, r;
    capture_stderr_start(&s, &r);

    char *p = alloc_mem(32);
    ASSERT_NOT_NULL("p should not be null", p);

    realloc_mem(p + 3, 40);

    char *out = capture_stderr_end(s, r);
    ASSERT_STR_MATCH("realloc_mem for offset pointer should log error", out, "heap corruption");
    free(out);
}

TEST(realloc_mem_unallocated_heap_logs_error) {
    int s, r;
    capture_stderr_start(&s, &r);

    char *p = alloc_mem(32);
    ASSERT_NOT_NULL("p should not be null", p);

    char *pp = (char *) sbrk(0);

    realloc_mem(pp - 10, 40);

    char *out = capture_stderr_end(s, r);
    ASSERT_STR_MATCH("realloc_mem for unallocated heap address should log error", out,
                     "heap corruption");
    free(out);
}

TEST(realloc_mem_freed_pointer_logs_error) {
    char *p = alloc_mem(32);
    ASSERT_NOT_NULL("p should not be null", p);

    int s, r;
    capture_stderr_start(&s, &r);

    free_mem(p);
    realloc_mem(p, 40);

    char *out = capture_stderr_end(s, r);
    ASSERT_STR_MATCH("realloc_mem for freed pointer should log error", out, "double free");
    free(out);
}
