/**
 * @file
 * @brief Own real-file semantic fixtures and byte-exact diagnostic comparison.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../docs/implementation/library-tests-and-build.md)
 */
#ifndef AMA_SEMANTIC_INTEGRATION_COMMON
#define AMA_SEMANTIC_INTEGRATION_COMMON

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "semantic/semantic.h"
#include "diagnostics/render/render.h"

static Project project;
static SemanticProgram semantic;

/**
 * Initialize this leaf fixture before its assertions; the enclosing file contract identifies
 * owned streams, source storage or model state. Setup failure is an assertion, never an accepted
 * product failure.
 */
static void setup_test(void) {
    SourceAllocator allocator = source_system_allocator();
    project_init(&project, allocator);
    ModuleRoot root = {"project", NULL, 0};
    ProjectStatus loaded = project_load(&project, &root, 1, ".main");
    assert(loaded == PROJECT_OK);
    semantic_program_init(&semantic, &project, allocator);
}

/**
 * Release or reset this leaf fixture after observations, in reverse borrow order. Counted owners
 * assert zero remaining live allocations; assertions remain enabled by the leaf harness.
 */
static void destroy_test(void) {
    semantic_program_destroy(&semantic);
    project_destroy(&project);
}

/**
 * Observe the real structured or captured runtime diagnostic using this fixture contract.
 * Require the expected bytes/text and complete IO rather than relying on exit failure alone.
 */
static void expect_diagnostic(void) {
    FILE *actual = tmpfile();
    FILE *expected = fopen("expected-error.txt", "rb");
    assert(actual != NULL && expected != NULL);
    DiagnosticRenderStatus status = diagnostic_render(actual, &semantic.error);
    assert(status == DIAGNOSTIC_RENDER_OK);
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
