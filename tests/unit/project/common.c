/**
 * @file
 * @brief Own counted project closure fixtures and checked physical source creation.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../docs/implementation/library-tests-and-build.md)
 */
#ifndef AMA_PROJECT_TEST_COMMON
#define AMA_PROJECT_TEST_COMMON

#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include "../source/common.c"
#include "project/dependencies/project.h"

static Project project;

/**
 * Initialize the counted source fixture and project closure owner before loading one explicit
 * root.
 */
static void setup_project(void) {
    setup_test();
    project_init(&project, sources.allocator);
}

/**
 * Destroy project graphs/source catalog before the allocator fixture and assert complete
 * release.
 */
static void destroy_project(void) {
    project_destroy(&project);
    destroy_test();
}

/**
 * Create a test-owned fixture directory or accept its existing directory setup as specified by
 * the caller; other mkdir errors fail assertions.
 */
static void directory(const char *path) {
    int created = mkdir(path, 0700);
    assert(created == 0 || errno == EEXIST);
}

/**
 * Write exact supplied source text into a fixture path, checking full byte count and close
 * before loading it.
 */
static void file(const char *path, const char *content) {
    FILE *stream = fopen(path, "wb");
    assert(stream != NULL);
    size_t length = strlen(content);
    size_t written = fwrite(content, 1, length, stream);
    int closed = fclose(stream);
    assert(written == length && closed == 0);
}

/**
 * Load .main from exactly the supplied physical root through the production closure loader.
 */
static ProjectStatus load_root(const char *root) {
    ModuleRoot input = {root, NULL, 0};
    return project_load(&project, &input, 1, ".main");
}

/**
 * Count actual loaded module records for closure assertions, not physical files or duplicate
 * dependency occurrences.
 */
static size_t module_count(void) {
    size_t count = 0;
    for (ProjectModule *module = project.first; module != NULL; module = module->next) {
        count++;
    }
    return count;
}

#endif
