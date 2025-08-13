#include "test.h"
#include <stdio.h>
#include <string.h>

#define MAX_TESTS 1024

static test_entry tests[MAX_TESTS];
static int test_count = 0;

void register_test(const char *file, const char *name, test_func func) {
    if (test_count < MAX_TESTS) {
        tests[test_count].file = file;
        tests[test_count].name = name;
        tests[test_count].func = func;
        test_count++;
    } else {
        fprintf(stderr, "Too many tests registered!\n");
    }
}

void run_all_tests(const char *filter) {
    printf("Discoverd %d tests. Running...\n", test_count);
    int run = 0;
    for (int i = 0; i < test_count; i++) {
        if (filter && !strstr(tests[i].name, filter))
            continue;
        printf("[RUNNING] %s - %s\n", tests[i].file, tests[i].name);
        tests[i].func();
        run++;
    }

    printf("Done. Run %d/%d tests\n", run, test_count);
}
