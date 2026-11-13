/**
 * @file
 * @brief Own scaffold arena, exact identities, source bytes and first-error lifetime.
 *
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * [ERROR-04](../../../../spec/implementation/diagnostics.md#error-04) provider
 * rows borrow registered identities and must be destroyed before that registry.
 * The validated frontend remains borrowed; failed model/output storage is released by one explicit owner.
 */
#include <assert.h>
#include <string.h>
#include "native/scaffold/internal.h"

void native_scaffold_init(NativeScaffold *scaffold, const CompilerFrontend *frontend,
    SourceAllocator allocator) {
    assert(frontend->valid);
    *scaffold = (NativeScaffold){.frontend = frontend};
    source_arena_init(&scaffold->arena, allocator);
    identity_registry_init(&scaffold->identities, allocator);
    native_status_providers_init(&scaffold->providers, allocator);
    source_manager_init(&scaffold->texts, allocator);
}

void native_scaffold_destroy(NativeScaffold *scaffold) {
    source_manager_destroy(&scaffold->texts);
    native_status_providers_destroy(&scaffold->providers);
    identity_registry_destroy(&scaffold->identities);
    source_arena_destroy(&scaffold->arena);
    *scaffold = (NativeScaffold){0};
}

bool scaffold_error(NativeScaffold *scaffold, DiagnosticCode code, const char *message, const char *path) {
    if (scaffold->error.message == NULL) {
        scaffold->error = (Diagnostic){.code = code, .message = message, .primary = {.path = path}};
    }
    return false;
}

void *scaffold_allocate(NativeScaffold *scaffold, size_t size) {
    void *allocation = source_arena_allocate(&scaffold->arena, size);
    if (allocation == NULL) {
        scaffold_error(scaffold, DIAGNOSTIC_OUT_OF_MEMORY, "Native scaffolding could not allocate storage", NULL);
    } else {
        memset(allocation, 0, size);
    }
    return allocation;
}

const char *scaffold_join(NativeScaffold *scaffold, const char *left, const char *separator, const char *right) {
    const char *result = source_arena_join(&scaffold->arena, left, separator, right);
    if (result == NULL) {
        scaffold_error(scaffold, DIAGNOSTIC_OUT_OF_MEMORY, "Native scaffolding could not allocate a path", NULL);
    }
    return result;
}
