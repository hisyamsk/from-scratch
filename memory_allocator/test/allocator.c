#include "allocator.h"
#include "test.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

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
    ASSERT_NOT_EQUAL("a and b should not be equal", a, b);
    ASSERT_TRUE("a and b should not overlap", (a + 10 <= b) || (b + 10 <= a));
}

TEST(alloc_mem_alignment) {
    void *ptr = alloc_mem(1);
    ASSERT_NOT_NULL("ptr should not be null", ptr);
    ASSERT_EQUAL("pointer should be aligned", 0, (uintptr_t) ptr % sizeof(void *));
}
