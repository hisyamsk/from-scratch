#ifndef TEST_H
#define TEST_H

typedef void (*test_func)(void);

typedef struct {
    const char *file;
    const char *name;
    test_func func;
} test_entry;

void register_test(const char *file, const char *name, test_func t);
void run_all_tests(const char *filter);

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
            printf("❌ Failed: %s\n", msg);                                                        \
        }                                                                                          \
    } while (0)

#endif
