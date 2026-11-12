/**
 * @file
 * @brief Own copied recursive-harness fixtures and verify preserved failure artifacts.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../docs/implementation/library-tests-and-build.md)
 */
#ifndef AMA_TEST_HARNESS_COMMON_C
#define AMA_TEST_HARNESS_COMMON_C

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../support/files.c"
#include "support/process.c"

/**
 * Use lstat to distinguish an existing fixture path from ENOENT; other filesystem failures are
 * test failures.
 */
static int path_exists(const char *path) {
    struct stat information;
    int result = lstat(path, &information);
    assert(result == 0 || errno == ENOENT);
    return result == 0;
}

/**
 * Compare a complete bounded fixture file with the expected exact bytes, checking read and close
 * completion.
 */
static void expect_contents(const char *path, const char *expected) {
    FILE *file = fopen(path, "r");
    assert(file != NULL);
    char contents[256];
    size_t length = fread(contents, 1, sizeof(contents), file);
    assert(!ferror(file));
    assert(length == strlen(expected));
    assert(memcmp(contents, expected, length) == 0);
    int result = fclose(file);
    assert(result == 0);
}

/**
 * Copy the actual maintained build rules into an isolated project before changing its fixture
 * inputs; never mutate the repository rules under test.
 */
static void copy_rules(void) {
    copy_file(TEST_ROOT "/make/configuration.mk", "generated/project/make/configuration.mk");
    copy_file(TEST_ROOT "/tests/parent.mk", "generated/project/tests/parent.mk");
    copy_file(TEST_ROOT "/tests/leaf.mk", "generated/project/tests/leaf.mk");
}

/**
 * Copy the explicitly enumerated fixture inputs into test-owned generated storage; all path
 * construction and copying is checked.
 */
static void copy_fixture(void) {
    const char *names[] = {
        "Makefile", "common.c", "left/Makefile", "right/Makefile", "left/main.c", "right/main.c"
    };
    for (size_t index = 0; index < sizeof(names) / sizeof(names[0]); index++) {
        char source[256];
        char destination[256];
        int length = snprintf(source, sizeof(source), "project/tests/%s", names[index]);
        assert(length > 0 && (size_t) length < sizeof(source));
        length = snprintf(destination, sizeof(destination),
            "generated/project/tests/%s", names[index]);
        assert(length > 0 && (size_t) length < sizeof(destination));
        copy_file(source, destination);
    }
}

/**
 * Initialize this leaf fixture before its assertions; the enclosing file contract identifies
 * owned streams, source storage or model state. Setup failure is an assertion, never an accepted
 * product failure.
 */
static void setup_test(void) {
    const char *directories[] = {
        "generated/project", "generated/project/make", "generated/project/generated",
        "generated/project/tests", "generated/project/tests/left", "generated/project/tests/right"
    };
    for (size_t index = 0; index < sizeof(directories) / sizeof(directories[0]); index++) {
        make_directory(directories[index]);
    }
    copy_rules();
    copy_fixture();
}

/**
 * Release or reset this leaf fixture after observations, in reverse borrow order. Counted owners
 * assert zero remaining live allocations; assertions remain enabled by the leaf harness.
 */
static void destroy_test(void) {
    remove_tree("generated/project");
}

/**
 * Require the failed leaf binary/started marker and exact setup/run trace to remain available;
 * successful cleanup would incorrectly erase this evidence.
 */
static void expect_failure_outputs(void) {
    assert(path_exists("generated/project/tests/left/generated/test"));
    assert(path_exists("generated/project/tests/left/generated/started"));
    expect_contents("generated/project/generated/left", "setup\nrun\n");
}

#endif
