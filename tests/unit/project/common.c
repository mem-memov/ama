#ifndef AMA_PROJECT_TEST_COMMON
#define AMA_PROJECT_TEST_COMMON

#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include "../source/common.c"
#include "project/dependencies/project.h"

static Project project;

static void setup_project(void) {
    setup_test();
    project_init(&project, sources.allocator);
}

static void destroy_project(void) {
    project_destroy(&project);
    destroy_test();
}

static void directory(const char *path) {
    int created = mkdir(path, 0700);
    assert(created == 0 || errno == EEXIST);
}

static void file(const char *path, const char *content) {
    FILE *stream = fopen(path, "wb");
    assert(stream != NULL);
    size_t length = strlen(content);
    size_t written = fwrite(content, 1, length, stream);
    int closed = fclose(stream);
    assert(written == length && closed == 0);
}

static ProjectStatus load_root(const char *root) {
    ModuleRoot input = {root, NULL, 0};
    return project_load(&project, &input, 1, ".main");
}

static size_t module_count(void) {
    size_t count = 0;
    for (ProjectModule *module = project.first; module != NULL; module = module->next) {
        count++;
    }
    return count;
}

#endif
