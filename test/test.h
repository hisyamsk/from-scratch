#ifndef TEST_H
#define TEST_H

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef void (*test_func)(void);

typedef struct {
    const char *file;
    const char *name;
    test_func func;
} test_entry_t;

void run_tests(int argc, char **argv);
void register_test(const char *file, const char *name, test_func func);
void fail_test(void);

void capture_stderr_start(int *saved_stderr_fd, int *read_fd);
char *capture_stderr_end(int saved_stderr_fd, int read_fd);

#define TEST(name)                                                                                 \
    static void name(void);                                                                        \
    __attribute__((constructor)) static void register_##name(void) {                               \
        register_test(__FILE__, #name, name);                                                      \
    }                                                                                              \
    static void name(void)

#define ASSERT_TRUE(msg, cond)                                                                     \
    do {                                                                                           \
        if (cond) {                                                                                \
            printf("✅ Passed: %s\n", msg);                                                        \
        } else {                                                                                   \
            printf("❌ Failed: %s. Expected: true, Got: false\n", msg);                            \
            fail_test();                                                                           \
        }                                                                                          \
    } while (0);

#define ASSERT_FALSE(msg, cond)                                                                    \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            printf("✅ Passed: %s\n", msg);                                                        \
        } else {                                                                                   \
            printf("❌ Failed: %s. Expected: false, Got: true\n", msg);                            \
            fail_test();                                                                           \
        }                                                                                          \
    } while (0);

#define ASSERT_NOT_NULL(msg, ptr)                                                                  \
    do {                                                                                           \
        if ((ptr) != NULL) {                                                                       \
            printf("✅ Passed: %s\n", msg);                                                        \
        } else {                                                                                   \
            printf("❌ Failed: %s. Expected: not NULL. Got: NULL\n", msg);                         \
            fail_test();                                                                           \
        }                                                                                          \
    } while (0);

#define ASSERT_NULL(msg, ptr)                                                                      \
    do {                                                                                           \
        if ((ptr) == NULL) {                                                                       \
            printf("✅ Passed: %s\n", msg);                                                        \
        } else {                                                                                   \
            printf("❌ Failed: %s. Expected: NULL. Got: %p\n", msg, (ptr));                        \
            fail_test();                                                                           \
        }                                                                                          \
    } while (0);

#define ASSERT_INT_EQUAL(msg, expected, actual)                                                    \
    do {                                                                                           \
        if ((expected) == (actual)) {                                                              \
            printf("✅ Passed: %s\n", msg);                                                        \
        } else {                                                                                   \
            printf("❌ Failed: %s. Expected: %d. Got: %d\n", msg, (expected), (actual));           \
            fail_test();                                                                           \
        }                                                                                          \
    } while (0)

#define ASSERT_LONG_EQUAL(msg, expected, actual)                                                   \
    do {                                                                                           \
        if ((expected) == (actual)) {                                                              \
            printf("✅ Passed: %s\n", msg);                                                        \
        } else {                                                                                   \
            printf("❌ Failed: %s. Expected: %ld. Got: %ld\n", msg, (expected), (actual));         \
            fail_test();                                                                           \
        }                                                                                          \
    } while (0)

#define ASSERT_LLONG_EQUAL(msg, expected, actual)                                                  \
    do {                                                                                           \
        if ((expected) == (actual)) {                                                              \
            printf("✅ Passed: %s\n", msg);                                                        \
        } else {                                                                                   \
            printf("❌ Failed: %s. Expected: %lld. Got: %lld\n", msg, (expected), (actual));       \
            fail_test();                                                                           \
        }                                                                                          \
    } while (0)

#define ASSERT_UINT_EQUAL(msg, expected, actual)                                                   \
    do {                                                                                           \
        if ((expected) == (actual)) {                                                              \
            printf("✅ Passed: %s\n", msg);                                                        \
        } else {                                                                                   \
            printf("❌ Failed: %s. Expected: %u. Got: %u\n", msg, (expected), (actual));           \
            fail_test();                                                                           \
        }                                                                                          \
    } while (0)

#define ASSERT_ULONG_EQUAL(msg, expected, actual)                                                  \
    do {                                                                                           \
        if ((expected) == (actual)) {                                                              \
            printf("✅ Passed: %s\n", msg);                                                        \
        } else {                                                                                   \
            printf("❌ Failed: %s. Expected: %lu. Got: %lu\n", msg, (expected), (actual));         \
            fail_test();                                                                           \
        }                                                                                          \
    } while (0)

#define ASSERT_UINTPTR_EQUAL(msg, expected, actual)                                                \
    do {                                                                                           \
        if ((expected) == (actual)) {                                                              \
            printf("✅ Passed: %s\n", msg);                                                        \
        } else {                                                                                   \
            printf("❌ Failed: %s. Expected: %ju. Got: %ju\n", msg, (uintmax_t) (expected),        \
                   (uintmax_t) (actual));                                                          \
            fail_test();                                                                           \
        }                                                                                          \
    } while (0)

#define ASSERT_INT_NOT_EQUAL(msg, expected, actual)                                                \
    do {                                                                                           \
        if ((expected) != (actual)) {                                                              \
            printf("✅ Passed: %s\n", msg);                                                        \
        } else {                                                                                   \
            printf("❌ Failed: %s. Both values were %d (unexpected equal)\n", msg, (expected));    \
            fail_test();                                                                           \
        }                                                                                          \
    } while (0)

#define ASSERT_LONG_NOT_EQUAL(msg, expected, actual)                                               \
    do {                                                                                           \
        if ((expected) != (actual)) {                                                              \
            printf("✅ Passed: %s\n", msg);                                                        \
        } else {                                                                                   \
            printf("❌ Failed: %s. Both values were %ld (unexpected equal)\n", msg, (expected));   \
            fail_test();                                                                           \
        }                                                                                          \
    } while (0)

#define ASSERT_LLONG_NOT_EQUAL(msg, expected, actual)                                              \
    do {                                                                                           \
        if ((expected) != (actual)) {                                                              \
            printf("✅ Passed: %s\n", msg);                                                        \
        } else {                                                                                   \
            printf("❌ Failed: %s. Both values were %lld (unexpected equal)\n", msg, (expected));  \
            fail_test();                                                                           \
        }                                                                                          \
    } while (0)

#define ASSERT_UINT_NOT_EQUAL(msg, expected, actual)                                               \
    do {                                                                                           \
        if ((expected) != (actual)) {                                                              \
            printf("✅ Passed: %s\n", msg);                                                        \
        } else {                                                                                   \
            printf("❌ Failed: %s. Both values were %u (unexpected equal)\n", msg, (expected));    \
            fail_test();                                                                           \
        }                                                                                          \
    } while (0)

#define ASSERT_ULONG_NOT_EQUAL(msg, expected, actual)                                              \
    do {                                                                                           \
        if ((expected) != (actual)) {                                                              \
            printf("✅ Passed: %s\n", msg);                                                        \
        } else {                                                                                   \
            printf("❌ Failed: %s. Both values were %lu (unexpected equal)\n", msg, (expected));   \
            fail_test();                                                                           \
        }                                                                                          \
    } while (0)

#define ASSERT_UINTPTR_NOT_EQUAL(msg, expected, actual)                                            \
    do {                                                                                           \
        if ((expected) != (actual)) {                                                              \
            printf("✅ Passed: %s\n", msg);                                                        \
        } else {                                                                                   \
            printf("❌ Failed: %s. Both values were %ju (unexpected equal)\n", msg,                \
                   (uintmax_t) (expected));                                                        \
            fail_test();                                                                           \
        }                                                                                          \
    } while (0)

#define ASSERT_PTR_EQUAL(msg, expected, actual)                                                    \
    do {                                                                                           \
        void *_pa = (expected);                                                                    \
        void *_pb = (actual);                                                                      \
        if (_pa == _pb) {                                                                          \
            printf("✅ Passed: %s\n", msg);                                                        \
        } else {                                                                                   \
            printf("❌ Failed: %s. Expected: %p. Got: %p\n", msg, (expected), (actual));           \
            fail_test();                                                                           \
        }                                                                                          \
    } while (0);

#define ASSERT_PTR_NOT_EQUAL(msg, expected, actual)                                                \
    do {                                                                                           \
        void *_pa = (expected);                                                                    \
        void *_pb = (actual);                                                                      \
        if (_pa != _pb) {                                                                          \
            printf("✅ Passed: %s\n", msg);                                                        \
        } else {                                                                                   \
            printf("❌ Failed: %s. Both pointers were \"%p\" (unexpected equal)\n", msg,           \
                   (expected));                                                                    \
            fail_test();                                                                           \
        }                                                                                          \
    } while (0);

#define ASSERT_MEM_EQUAL(msg, expected, actual, size)                                              \
    do {                                                                                           \
        if (memcmp((expected), (actual), (size)) == 0) {                                           \
            printf("✅ Passed: %s\n", msg);                                                        \
        } else {                                                                                   \
            for (size_t i = 0; i < (size); i++) {                                                  \
                if (((const unsigned char *) (expected))[i] !=                                     \
                    ((const unsigned char *) (actual))[i]) {                                       \
                    printf("❌ Failed: %s. Memory differs at byte %zu. Expected: 0x%02X, Got: "    \
                           "0x%02X\n",                                                             \
                           msg, i, ((const unsigned char *) (expected))[i],                        \
                           ((const unsigned char *) (actual))[i]);                                 \
                    break;                                                                         \
                }                                                                                  \
            }                                                                                      \
            fail_test();                                                                           \
        }                                                                                          \
    } while (0)

#define ASSERT_MEM_NOT_EQUAL(msg, expected, actual, size)                                          \
    do {                                                                                           \
        if (memcmp((expected), (actual), (size)) != 0) {                                           \
            printf("✅ Passed: %s\n", msg);                                                        \
        } else {                                                                                   \
            printf("❌ Failed: %s. Memory regions are identical (%zu bytes)\n", msg,               \
                   (size_t) (size));                                                               \
            fail_test();                                                                           \
        }                                                                                          \
    } while (0)

#define ASSERT_STR_EQUAL(msg, expected, actual, size)                                              \
    do {                                                                                           \
        if (strncmp((expected), (actual), (size)) == 0) {                                          \
            printf("✅ Passed: %s\n", msg);                                                        \
        } else {                                                                                   \
            printf("❌ Failed: %s. Expected: %s. Got: %s\n", msg, expected, actual);               \
            fail_test();                                                                           \
        }                                                                                          \
    } while (0);

#define ASSERT_STR_NOT_EQUAL(msg, expected, actual, size)                                          \
    do {                                                                                           \
        if (strncmp((expected), (actual), (size)) != 0) {                                          \
            printf("✅ Passed: %s\n", msg);                                                        \
        } else {                                                                                   \
            printf("❌ Failed: %s. Both strings were \"%s\" (unexpected equal)\n", msg,            \
                   (expected));                                                                    \
            fail_test();                                                                           \
        }                                                                                          \
    } while (0)

#define ASSERT_STR_MATCH(msg, haystack, needle)                                                    \
    do {                                                                                           \
        if (strstr(haystack, needle) != NULL) {                                                    \
            printf("✅ Passed: %s\n", msg);                                                        \
        } else {                                                                                   \
            printf("❌ Failed: %s. Pattern %s not found in %s\n", msg, needle, haystack);          \
            fail_test();                                                                           \
        }                                                                                          \
    } while (0)

#endif
