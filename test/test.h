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

extern int test_passed;
extern int test_failed;

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
            test_passed++;                                                                         \
        } else {                                                                                   \
            printf("❌ Failed: %s. Expected: true, Got: false\n", msg);                            \
            test_failed++;                                                                         \
        }                                                                                          \
    } while (0);

#endif
