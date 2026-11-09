#ifndef AMA_TESTS_COMMON_COMMON_C
#define AMA_TESTS_COMMON_COMMON_C
#include "../common.c"
#include "common/names.h"

static NameRegistry registry;

static void setup_test(void) {
    test_count++;
    error_clear(&error);
    registry = (NameRegistry){0};
}

static void destroy_test(void) {
    name_registry_destroy(&registry);
}
#endif
