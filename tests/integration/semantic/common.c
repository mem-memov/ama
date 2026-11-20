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
#include "compiler/frontend.h"
#include "diagnostics/render/render.h"

static CompilerFrontend frontend;
static bool checked;

/**
 * Initialize this leaf fixture before its assertions; the enclosing file contract identifies
 * owned streams, source storage or model state. Setup failure is an assertion, never an accepted
 * product failure.
 */
static void setup_test(void) {
    SourceAllocator allocator = source_system_allocator();
    compiler_frontend_init(&frontend,allocator);
    ModuleRoot root = {"project", NULL, 0, MODULE_ROOT_PROJECT};
    checked = compiler_frontend_check(&frontend,&root,1,"/test/fixture/v1.0.0/src/main");
}

/**
 * Release or reset this leaf fixture after observations, in reverse borrow order. Counted owners
 * assert zero remaining live allocations; assertions remain enabled by the leaf harness.
 */
static void destroy_test(void) {
    compiler_frontend_destroy(&frontend);
}

/**
 * Observe the real structured or captured runtime diagnostic using this fixture contract.
 * Require the expected bytes/text and complete IO rather than relying on exit failure alone.
 */
static void expect_diagnostic(void) {
    FILE *actual = tmpfile();
    FILE *expected = fopen("expected-error.txt", "rb");
    assert(actual != NULL && expected != NULL);
    DiagnosticRenderStatus status = diagnostic_render(actual, &frontend.error);
    assert(status == DIAGNOSTIC_RENDER_OK);
    rewind(actual);
    int left;
    int right;
    do {
        left = fgetc(actual);
        right = fgetc(expected);
        if (left != right) {
            rewind(actual);
            int byte;
            while ((byte = fgetc(actual)) != EOF) { assert(fputc(byte,stderr) != EOF); }
        }
        assert(left == right);
    } while (left != EOF);
    assert(!ferror(actual) && !ferror(expected));
    int closed_actual = fclose(actual);
    int closed_expected = fclose(expected);
    assert(closed_actual == 0 && closed_expected == 0);
}

#endif
