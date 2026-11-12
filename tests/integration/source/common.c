/**
 * @file
 * @brief Own source/catalog/identity fixtures and compare exact diagnostics and tool output.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../docs/implementation/library-tests-and-build.md)
 */
#ifndef AMA_SOURCE_INTEGRATION_COMMON
#define AMA_SOURCE_INTEGRATION_COMMON

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include "source/module/catalog.h"
#include "source/identity/identity.h"
#include "diagnostics/adapters/adapters.h"
#include "diagnostics/render/render.h"
#include "source/mapping/directive.h"

static ModuleCatalog catalog;
static IdentityRegistry identities;

/**
 * Initialize this leaf fixture before its assertions; the enclosing file contract identifies
 * owned streams, source storage or model state. Setup failure is an assertion, never an accepted
 * product failure.
 */
static void setup_test(void) {
    module_catalog_init(&catalog, source_system_allocator());
    identity_registry_init(&identities, source_system_allocator());
}

/**
 * Release or reset this leaf fixture after observations, in reverse borrow order. Counted owners
 * assert zero remaining live allocations; assertions remain enabled by the leaf harness.
 */
static void destroy_test(void) {
    identity_registry_destroy(&identities);
    module_catalog_destroy(&catalog);
}

/**
 * Compare both finite files byte for byte including EOF, checking read/close completion
 * independently of rendered text interpretation.
 */
static void compare_files(const char *actual_path, const char *expected_path) {
    FILE *actual = fopen(actual_path, "rb");
    FILE *expected = fopen(expected_path, "rb");
    assert(actual != NULL && expected != NULL);
    int actual_byte;
    int expected_byte;
    do {
        actual_byte = fgetc(actual);
        expected_byte = fgetc(expected);
        assert(actual_byte == expected_byte);
    } while (actual_byte != EOF);
    assert(!ferror(actual) && !ferror(expected));
    int closed_actual = fclose(actual);
    int closed_expected = fclose(expected);
    assert(closed_actual == 0 && closed_expected == 0);
}

/**
 * Observe the real structured or captured runtime diagnostic using this fixture contract.
 * Require the expected bytes/text and complete IO rather than relying on exit failure alone.
 */
static void expect_diagnostic(const Diagnostic *diagnostic, const char *expected) {
    FILE *output = fopen("generated/diagnostic.txt", "wb");
    assert(output != NULL);
    DiagnosticRenderStatus status = diagnostic_render(output, diagnostic);
    assert(status == DIAGNOSTIC_RENDER_OK);
    int closed = fclose(output);
    assert(closed == 0);
    compare_files("generated/diagnostic.txt", expected);
}

/**
 * Exec exact tool argv in a child and wait through EINTR, requiring normal zero exit. No shell
 * interpretation or guessed success is permitted.
 */
static void run_process(char *const arguments[]) {
    pid_t child = fork();
    assert(child >= 0);
    if (child == 0) {
        execvp(arguments[0], arguments);
        _exit(127);
    }
    int status;
    pid_t waited;
    do {
        waited = waitpid(child, &status, 0);
    } while (waited < 0 && errno == EINTR);
    assert(waited == child && WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

#endif
