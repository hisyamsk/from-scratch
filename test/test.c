#define _GNU_SOURCE
#include "test.h"
#include <fcntl.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAX_TESTS 1024

static test_entry_t tests[MAX_TESTS];
static int test_count = 0;

int test_passed = 0;
int test_failed = 0;

jmp_buf test_env;
static int current_failed = 0;

void register_test(const char *file, const char *name, test_func func) {
    if (test_count < MAX_TESTS) {
        tests[test_count].file = file;
        tests[test_count].name = name;
        tests[test_count].func = func;
        test_count++;
    } else {
        fprintf(stderr, "register_test: Too many tests registered!\n");
    }
}

static int matches_filter(const test_entry_t *t, int argc, char **argv) {
    if (argc <= 1)
        return 1;
    for (int i = 1; i < argc; i++) {
        const char *pattern = argv[i];
        if (strstr(t->file, pattern) || strstr(t->name, pattern))
            return 1;
    }

    return 0;
}

void run_tests(int argc, char **argv) {
    int run = 0;

    printf("[==========] 🔍 Discovered %d tests.\n\n", test_count);
    for (int i = 0; i < test_count; i++) {
        if (!matches_filter(&tests[i], argc, argv))
            continue;

        printf("[RUNNING] %s - %s\n", tests[i].file, tests[i].name);
        current_failed = 0;
        if (setjmp(test_env) == 0) {
            tests[i].func();

            if (!current_failed)
                test_passed++;
        } else {
            test_failed++;
        }
        run++;
    }

    printf("\n");
    printf("[==========] 📋 Done. Ran %d/%d tests. Passed: %d. Failed: %d\n", run, test_count,
           test_passed, test_failed);
}

void fail_test() {
    current_failed = 1;
    longjmp(test_env, 1);
}

void capture_stderr_start(int *saved_stderr_fd, int *read_fd) {
    int pipefd[2];
    pipe(pipefd);

    fflush(stderr);

    *read_fd = pipefd[0];
    *saved_stderr_fd = dup(fileno(stderr));

    dup2(pipefd[1], fileno(stderr));
    close(pipefd[1]);
}

char *capture_stderr_end(int saved_stderr_fd, int read_fd) {
    fflush(stderr);
    dup2(saved_stderr_fd, fileno(stderr));
    close(saved_stderr_fd);

    char buf[4096];
    ssize_t n = read(read_fd, buf, sizeof(buf) - 1);
    close(read_fd);

    if (n < 0)
        return NULL;
    buf[n] = '\0';
    return strdup(buf);
}
