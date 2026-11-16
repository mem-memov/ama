/**
 * @file
 * @brief Own syntax-only inspection of one exact module source.
 *
 * [SYN-04](../../../spec/language/source-and-names.md#syn-04)
 * Draft inspection may succeed but does not certify executable dependency closure or semantics.
 */
#ifndef AMA_COMPILER_MODULE_INSPECTION_H
#define AMA_COMPILER_MODULE_INSPECTION_H

#include "ast/ast.h"
#include "diagnostics/adapters/adapters.h"

/**
 * Own catalog source storage and one structural AST plus diagnostic related storage. Successful
 * draft inspection is not executable project validation.
 * Component contract: [SYN-04](../../../spec/language/source-and-names.md#syn-04). */
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
/**
 * Initialize independent catalog and syntax owners using the explicit allocator; inspect at most
 * once.
 * Component contract: [SYN-04](../../../spec/language/source-and-names.md#syn-04). */
void compiler_inspection_init(ModuleInspection *inspection, SourceAllocator allocator);
/** Destroy syntax before catalog source storage, then reset diagnostics and attempt state. Component contract: [SYN-04](../../../spec/language/source-and-names.md#syn-04). */
void compiler_inspection_destroy(ModuleInspection *inspection);
/**
 * Build the exact catalog, find one logical path and structurally parse it. Draft may succeed;
 * no dependency closure or executable semantic acceptance is implied.
 * Component contract: [SYN-04](../../../spec/language/source-and-names.md#syn-04). */
bool compiler_inspect_module(ModuleInspection *inspection, const ModuleRoot *roots,
    size_t count, const char *logical_path);

#endif
