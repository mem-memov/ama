/**
 * @file
 * @brief Compose counted project/frontend fixtures and explicit source-file creation.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../docs/implementation/library-tests-and-build.md)
 */
#ifndef AMA_COMPILER_TEST_COMMON
#define AMA_COMPILER_TEST_COMMON

#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include "../source/common.c"
#include "compiler/frontend.h"

static CompilerFrontend frontend;

/**
 * Initialize counted source and frontend owners together before one check/authoring attempt.
 */
static void setup_frontend(void) {
    setup_test();
    compiler_frontend_init(&frontend, sources.allocator);
}

/**
 * Destroy the frontend before its source allocator fixture and require every counted allocation
 * to be released.
 */
static void destroy_frontend(void) {
    compiler_frontend_destroy(&frontend);
    destroy_test();
}

/**
 * Create a test-owned fixture directory or accept its existing directory setup as specified by
 * the caller; other mkdir errors fail assertions.
 */
static void directory(const char *path) {
    int result = mkdir(path, 0700);
    assert(result == 0 || errno == EEXIST);
}

/**
 * Write exact supplied source text into a fixture path, checking full byte count and close
 * before loading it.
 */
static void file(const char *path, const char *content) {
    FILE *output = fopen(path, "wb");
    assert(output != NULL);
    size_t count = strlen(content);
    size_t written = fwrite(content, 1, count, output);
    int closed = fclose(output);
    assert(written == count && closed == 0);
}

/**
 * Check the explicit generated project/main root through the production frontend; no native
 * compiler or executable is invoked.
 */
static inline bool check_project(void) {
    ModuleRoot root = {"generated/project", NULL, 0};
    return compiler_frontend_check(&frontend, &root, 1, ".main");
}

#endif
