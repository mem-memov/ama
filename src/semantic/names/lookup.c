/**
 * @file
 * @brief Resolve exact supported type paths and declaration names without normalization.
 *
 * [NAME-04](../../../spec/language/declarations.md#name-04)
 * [TPL-05](../../../spec/language/templates.md#tpl-05) makes a bare source path
 * insufficient when distinct required capture maps share one module origin.
 * These lookups consume the explicit inventory, not a spelling heuristic for syntax or inferred variables.
 * [TYPE-06](../../../spec/language/declarations.md#type-06) Inference introduces exact semantic names without manufacturing declarations.
 */
#include <string.h>
#include "semantic/internal.h"

SemanticType *semantic_type(const SemanticProgram *program, SourceSpan path) {
    SemanticType *selected = NULL;
    size_t length = path.end - path.start;
    for (SemanticType *type = program->first; type != NULL; type = type->next) {
        const char *name = type->module->source->logical_path;
        if (strlen(name) == length && memcmp(name, source_bytes(path.source) + path.start, length) == 0) {
            if (selected) { return NULL; }
            selected = type;
        }
    }
    return selected;
}

SemanticSymbol *semantic_symbol(const SemanticType *type, SourceSpan name) {
    for (SemanticSymbol *symbol = type->first; symbol != NULL; symbol = symbol->next) {
        if (semantic_same_text(symbol->name, name)) {
            return symbol;
        }
    }
    return NULL;
}
