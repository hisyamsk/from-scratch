#include "test.h"
#include <stddef.h>

int main(int argc, char **argv) {
    const char *filter = (argc > 1) ? argv[1] : NULL;
    run_all_tests(filter);
    return 0;
}
