#ifndef AMA_COMPILER_MODULE_INSPECTION_H
#define AMA_COMPILER_MODULE_INSPECTION_H

#include "ast/ast.h"
#include "diagnostics/adapters/adapters.h"

typedef struct {
    ModuleCatalog catalog;
    AstModule syntax;
    Diagnostic error;
    DiagnosticRelated related;
    bool attempted;
    bool valid;
} ModuleInspection;

/* Index exact roots, then parse one source for activation/syntax only. A draft
 * is a successful inspection, never executable project validation. Own all
 * diagnostics/source views until destruction; inspect once. */
void compiler_inspection_init(ModuleInspection *inspection, SourceAllocator allocator);
void compiler_inspection_destroy(ModuleInspection *inspection);
bool compiler_inspect_module(ModuleInspection *inspection, const ModuleRoot *roots,
    size_t count, const char *logical_path);

#endif
