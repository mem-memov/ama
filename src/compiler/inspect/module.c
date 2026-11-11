#include <assert.h>
#include "module.h"

void compiler_inspection_init(ModuleInspection *inspection, SourceAllocator allocator) {
    *inspection = (ModuleInspection){0};
    module_catalog_init(&inspection->catalog, allocator);
    ast_module_init(&inspection->syntax, allocator);
}

void compiler_inspection_destroy(ModuleInspection *inspection) {
    ast_module_destroy(&inspection->syntax);
    module_catalog_destroy(&inspection->catalog);
    *inspection = (ModuleInspection){0};
}

bool compiler_inspect_module(ModuleInspection *inspection, const ModuleRoot *roots,
    size_t count, const char *logical_path) {
    assert(!inspection->attempted);
    inspection->attempted = true;
    ModuleStatus indexed = module_catalog_build(&inspection->catalog, roots, count);
    if (indexed != MODULE_OK) {
        bool adapted = diagnostic_from_module(&inspection->catalog.issue, &inspection->related, &inspection->error);
        assert(adapted);
        (void)adapted;
        return false;
    }
    const ModuleSource *source = NULL;
    if (module_catalog_find(&inspection->catalog, logical_path, &source) != MODULE_OK) {
        const char *owned = source_arena_copy(&inspection->catalog.arena, logical_path);
        inspection->error = (Diagnostic){.code = owned == NULL ? DIAGNOSTIC_OUT_OF_MEMORY : DIAGNOSTIC_MISSING_MODULE,
            .message = owned == NULL ? "Module inspection could not allocate storage" : "Module source was not found",
            .subject = owned};
        return false;
    }
    if (parser_module_parse(&inspection->syntax, source->text) != PARSE_OK) {
        inspection->error = inspection->syntax.error.diagnostic;
        return false;
    }
    inspection->valid = true;
    return true;
}
