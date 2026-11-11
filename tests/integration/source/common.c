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

static void setup_test(void) {
    module_catalog_init(&catalog, source_system_allocator());
    identity_registry_init(&identities, source_system_allocator());
}

static void destroy_test(void) {
    identity_registry_destroy(&identities);
    module_catalog_destroy(&catalog);
}

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

static void expect_diagnostic(const Diagnostic *diagnostic, const char *expected) {
    FILE *output = fopen("generated/diagnostic.txt", "wb");
    assert(output != NULL);
    DiagnosticRenderStatus status = diagnostic_render(output, diagnostic);
    assert(status == DIAGNOSTIC_RENDER_OK);
    int closed = fclose(output);
    assert(closed == 0);
    compare_files("generated/diagnostic.txt", expected);
}

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
