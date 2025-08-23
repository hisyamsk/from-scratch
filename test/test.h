#ifndef TEST_H
#define TEST_H

typedef void (*test_func)(void);

typedef struct {
    const char *file;
    const char *name;
    test_func func;
} test_entry;

void run_tests(int argc, char **argv);
void register_test(const char *file, const char *name, test_func func);
void fail_test(void);

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
            printf("❌ Failed: %s. Expected: !NULL. Got: NULL\n", msg);                            \
            fail_test();                                                                           \
        }                                                                                          \
    } while (0);

#define ASSERT_NULL(msg, ptr)                                                                      \
    do {                                                                                           \
        if ((ptr) == NULL) {                                                                       \
            printf("✅ Passed: %s\n", msg);                                                        \
        } else {                                                                                   \
            printf("❌ Failed: %s. Expected: NULL. Got: !NULL\n", msg);                            \
            fail_test();                                                                           \
        }                                                                                          \
    } while (0);

#define ASSERT_EQUAL(msg, a, b)                                                                    \
    do {                                                                                           \
        if ((a) == (b)) {                                                                          \
            printf("✅ Passed: %s\n", msg);                                                        \
        } else {                                                                                   \
            printf("❌ Failed: %s. Expected: Equal. Got: Not Equal\n", msg);                       \
            fail_test();                                                                           \
        }                                                                                          \
    } while (0);

#define ASSERT_NOT_EQUAL(msg, a, b)                                                                \
    do {                                                                                           \
        if ((a) != (b)) {                                                                          \
            printf("✅ Passed: %s\n", msg);                                                        \
        } else {                                                                                   \
            printf("❌ Failed: %s. Expected: Not Equal. Got: Equal\n", msg);                       \
            fail_test();                                                                           \
        }                                                                                          \
    } while (0);

#define ASSERT_PTR_EQUAL(msg, a, b)                                                                \
    do {                                                                                           \
        void *_pa = (a);                                                                           \
        void *_pb = (b);                                                                           \
        if (_pa == _pb) {                                                                          \
            printf("✅ Passed: %s\n", msg);                                                        \
        } else {                                                                                   \
            printf("❌ Failed: %s. Expected: Equal. Got: Not Equal\n", msg);                       \
            fail_test();                                                                           \
        }                                                                                          \
    } while (0);

#define ASSERT_MEM_EQUAL(msg, p1, p2, size)                                                        \
    do {                                                                                           \
        if (memcmp((p1), (p2), (size)) == 0) {                                                     \
            printf("✅ Passed: %s\n", msg);                                                        \
        } else {                                                                                   \
            printf("❌ Failed: %s. Expected: Equal. Got: Not Equal\n", msg);                       \
            fail_test();                                                                           \
        }                                                                                          \
    } while (0);

#define ASSERT_STR_EQUAL(msg, s1, s2, size)                                                        \
    do {                                                                                           \
        if (strncmp((s1), (s2), (size)) == 0) {                                                    \
            printf("✅ Passed: %s\n", msg);                                                        \
        } else {                                                                                   \
            printf("❌ Failed: %s. Expected: Equal. Got: Not Equal\n", msg);                       \
            fail_test();                                                                           \
        }                                                                                          \
    } while (0);

#endif
