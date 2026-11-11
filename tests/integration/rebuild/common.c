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

#include "support/files.c"
#include "support/process.c"
#include "support/trace.c"

static void copy_build_rules(void) {
    copy_file(TEST_ROOT "/Makefile", "generated/project/Makefile");
    copy_file(TEST_ROOT "/make/sources.mk", "generated/project/make/sources.mk");
    copy_file(TEST_ROOT "/make/configuration.mk", "generated/project/make/configuration.mk");
    copy_file(TEST_ROOT "/tests/leaf.mk", "generated/project/tests/leaf.mk");
}

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

static void prepare_build(const char *directory) {
    setup_test();
    copy_fixture();
    run_build(directory, "build", NULL);
    reset_trace();
}

static void prepare_project(void) {
    setup_test();
    copy_fixture();
    run_build("generated/project", "all", NULL);
    reset_trace();
}

static void destroy_test(void) {
    remove_tree("generated/project");
}

#endif
