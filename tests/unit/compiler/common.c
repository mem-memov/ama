#ifndef AMA_COMPILER_TEST_COMMON
#define AMA_COMPILER_TEST_COMMON

#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include "../source/common.c"
#include "compiler/frontend.h"

static CompilerFrontend frontend;

static void setup_frontend(void) {
    setup_test();
    compiler_frontend_init(&frontend, sources.allocator);
}

static void destroy_frontend(void) {
    compiler_frontend_destroy(&frontend);
    destroy_test();
}

static void directory(const char *path) {
    int result = mkdir(path, 0700);
    assert(result == 0 || errno == EEXIST);
}

static void file(const char *path, const char *content) {
    FILE *output = fopen(path, "wb");
    assert(output != NULL);
    size_t count = strlen(content);
    size_t written = fwrite(content, 1, count, output);
    int closed = fclose(output);
    assert(written == count && closed == 0);
}

static bool check_project(void) {
    ModuleRoot root = {"generated/project", NULL, 0};
    return compiler_frontend_check(&frontend, &root, 1, ".main", (SemanticLibraries){0});
}

#endif
