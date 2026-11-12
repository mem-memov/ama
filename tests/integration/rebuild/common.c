/**
 * @file
 * @brief Own copied build rules and projects for isolated incremental-build scenarios.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../docs/implementation/library-tests-and-build.md)
 */
#ifndef AMA_TEST_BUILD_COMMON_C
#define AMA_TEST_BUILD_COMMON_C

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static char working_directory[2048];
static char trace_path[4096];

#include "../support/files.c"
#include "support/process.c"
#include "support/trace.c"

/**
 * Copy the maintained compiler/test Make rules into isolated generated storage before exercising
 * incremental invalidation.
 */
static void copy_build_rules(void) {
    copy_file(TEST_ROOT "/Makefile", "generated/project/Makefile");
    copy_file(TEST_ROOT "/make/sources.mk", "generated/project/make/sources.mk");
    copy_file(TEST_ROOT "/make/configuration.mk", "generated/project/make/configuration.mk");
    copy_file(TEST_ROOT "/tests/leaf.mk", "generated/project/tests/leaf.mk");
}

/**
 * Initialize this leaf fixture before its assertions; the enclosing file contract identifies
 * owned streams, source storage or model state. Setup failure is an assertion, never an accepted
 * product failure.
 */
static void setup_test(void) {
    char *directory = getcwd(working_directory, sizeof(working_directory));
    assert(directory != NULL);
    int length = snprintf(trace_path, sizeof(trace_path),
                            "%s/generated/trace", working_directory);
    assert(length > 0 && (size_t) length < sizeof(trace_path));
    const char *directories[] = {
        "", "/make", "/src", "/src/nested", "/src/nested/detail", "/tests", "/tests/check"
    };
    for (size_t index = 0; index < sizeof(directories) / sizeof(directories[0]); index++) {
        char path[256];
        length = snprintf(path, sizeof(path), "generated/project%s", directories[index]);
        assert(length > 0 && (size_t) length < sizeof(path));
        make_directory(path);
    }
    copy_build_rules();
    reset_trace();
}

/**
 * Copy the explicitly enumerated fixture inputs into test-owned generated storage; all path
 * construction and copying is checked.
 */
static void copy_fixture(void) {
    const char *files[] = {
        "src/main.c", "src/nested/value.c", "src/nested/value.h",
        "src/nested/detail/constant.h", "src/unrelated.c", "tests/check/Makefile",
        "tests/check/main.c", "tests/common.c"
    };
    for (size_t index = 0; index < sizeof(files) / sizeof(files[0]); index++) {
        char source[256];
        char destination[256];
        int length = snprintf(source, sizeof(source), "project/%s", files[index]);
        assert(length > 0 && (size_t) length < sizeof(source));
        length = snprintf(destination, sizeof(destination), "generated/project/%s", files[index]);
        assert(length > 0 && (size_t) length < sizeof(destination));
        copy_file(source, destination);
    }
}

/**
 * Create the isolated build fixture and its exact compiler tracing environment; repository build
 * inputs remain untouched.
 */
static void prepare_build(const char *directory) {
    setup_test();
    copy_fixture();
    run_build(directory, "build", NULL);
    reset_trace();
}

/**
 * Write this scenario's explicit source/native fixtures in test-owned storage before loading or
 * compiling them; the caller controls their lifetime and expected result.
 */
static void prepare_project(void) {
    setup_test();
    copy_fixture();
    run_build("generated/project", "all", NULL);
    reset_trace();
}

/**
 * Release or reset this leaf fixture after observations, in reverse borrow order. Counted owners
 * assert zero remaining live allocations; assertions remain enabled by the leaf harness.
 */
static void destroy_test(void) {
    remove_tree("generated/project");
}

#endif
