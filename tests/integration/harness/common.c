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

static int path_exists(const char *path) {
    struct stat information;
    int result = lstat(path, &information);
    assert(result == 0 || errno == ENOENT);
    return result == 0;
}

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

static void copy_rules(void) {
    copy_file(TEST_ROOT "/make/configuration.mk", "generated/project/make/configuration.mk");
    copy_file(TEST_ROOT "/tests/parent.mk", "generated/project/tests/parent.mk");
    copy_file(TEST_ROOT "/tests/leaf.mk", "generated/project/tests/leaf.mk");
}

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

static void destroy_test(void) {
    remove_tree("generated/project");
}

static void expect_failure_outputs(void) {
    assert(path_exists("generated/project/tests/left/generated/test"));
    assert(path_exists("generated/project/tests/left/generated/started"));
    expect_contents("generated/project/generated/left", "setup\nrun\n");
}

#endif
