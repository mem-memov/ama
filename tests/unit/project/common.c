#ifndef AMA_TESTS_PROJECT_COMMON_C
#define AMA_TESTS_PROJECT_COMMON_C
#include "../common.c"
#include "project/project.h"
static void setup_test(void) {
    test_count++;
    error_clear(&error);
}
static void destroy_test(void) { }
#endif
