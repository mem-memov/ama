/**
 * @file
 * @brief Own real-file dependency loading fixtures and exact diagnostic comparison.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../docs/implementation/library-tests-and-build.md)
 */
#ifndef AMA_PROJECT_INTEGRATION_COMMON
#define AMA_PROJECT_INTEGRATION_COMMON

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "project/dependencies/project.h"
#include "diagnostics/render/render.h"

static Project project;

/**
 * Initialize this leaf fixture before its assertions; the enclosing file contract identifies
 * owned streams, source storage or model state. Setup failure is an assertion, never an accepted
 * product failure.
 */
static void setup_test(void) {
    project_init(&project, source_system_allocator());
}

/**
 * Release or reset this leaf fixture after observations, in reverse borrow order. Counted owners
 * assert zero remaining live allocations; assertions remain enabled by the leaf harness.
 */
static void destroy_test(void) {
    project_destroy(&project);
}

/**
 * Observe the real structured or captured runtime diagnostic using this fixture contract.
 * Require the expected bytes/text and complete IO rather than relying on exit failure alone.
 */
static void expect_diagnostic(const char *expected_path) {
    FILE *actual = tmpfile();
    FILE *expected = fopen(expected_path, "rb");
    assert(actual != NULL && expected != NULL);
    DiagnosticRenderStatus rendered = diagnostic_render(actual, &project.error);
    assert(rendered == DIAGNOSTIC_RENDER_OK);
    rewind(actual);
    int left;
    int right;
    do {
        left = fgetc(actual);
        right = fgetc(expected);
        assert(left == right);
    } while (left != EOF);
    assert(!ferror(actual) && !ferror(expected));
    int closed_actual = fclose(actual);
    int closed_expected = fclose(expected);
    assert(closed_actual == 0 && closed_expected == 0);
}

#endif
