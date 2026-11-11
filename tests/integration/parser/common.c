#ifndef AMA_PARSER_INTEGRATION_COMMON
#define AMA_PARSER_INTEGRATION_COMMON

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "source/module/catalog.h"
#include "ast/ast.h"
#include "parser/abi/abi.h"
#include "diagnostics/render/render.h"
#include "native/namespace/namespace.h"

static ModuleCatalog catalog;
static AstModule module;
static AbiTable table;
static IdentityRegistry identities;

static void setup_test(void) {
    SourceAllocator allocator = source_system_allocator();
    module_catalog_init(&catalog, allocator);
    ast_module_init(&module, allocator);
    abi_table_init(&table, allocator);
    identity_registry_init(&identities, allocator);
}

static void destroy_test(void) {
    abi_table_destroy(&table);
    identity_registry_destroy(&identities);
    ast_module_destroy(&module);
    module_catalog_destroy(&catalog);
}

static const ModuleSource *find_fixture(const char *logical_path) {
    ModuleRoot root = {"project", NULL, 0};
    ModuleStatus status = module_catalog_build(&catalog, &root, 1);
    assert(status == MODULE_OK);
    const ModuleSource *source;
    status = module_catalog_find(&catalog, logical_path, &source);
    assert(status == MODULE_OK);
    return source;
}

static void compare_output(const char *actual_path, const char *expected_path) {
    FILE *actual = fopen(actual_path, "rb");
    FILE *expected = fopen(expected_path, "rb");
    assert(actual != NULL && expected != NULL);
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

static void expect_error(const ParseError *error, const char *expected) {
    FILE *output = fopen("generated/error.txt", "wb");
    assert(output != NULL);
    DiagnosticRenderStatus rendered = diagnostic_render(output, &error->diagnostic);
    assert(rendered == DIAGNOSTIC_RENDER_OK);
    int closed = fclose(output);
    assert(closed == 0);
    compare_output("generated/error.txt", expected);
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
