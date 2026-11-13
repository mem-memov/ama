/**
 * @file
 * @brief Link exact nominal value types and check initializer visibility.
 *
 * [TYPE-01](../../../spec/language/declarations.md#type-01)
 * A constants-only namespace is not an instance type; source-order initializer visibility remains separate from loading cycles.
 * [TYPE-06](../../../spec/language/declarations.md#type-06) Inferred types already name exact known identities and need no invented type-path line.
 */
#include <string.h>
#include "semantic/internal.h"

SemanticType *semantic_find_type(const SemanticProgram *program, const char *logical_path) {
    if (logical_path != NULL) {
        for (SemanticType *type = program->first; type != NULL; type = type->next) {
            if (strcmp(type->module->source->logical_path, logical_path) == 0) {
                return type;
            }
        }
    }
    return NULL;
}

bool semantic_link_types(SemanticProgram *program) {
    for (SemanticType *owner = program->first; owner != NULL; owner = owner->next) {
        for (SemanticSymbol *symbol = owner->first; symbol != NULL; symbol = symbol->next) {
            if (symbol->inferred) { continue; }
            SourceSpan path = symbol->type_site;
            symbol->type = semantic_type(program, path);
            if (symbol->type == NULL || !symbol->type->executable) {
                return semantic_fail(program, path, "A value type must name an executable module", symbol);
            }
            if (symbol->role == SYMBOL_CONSTANT && symbol->reference) {
                return semantic_fail(program, symbol->spelling,
                    "Shared constants cannot use reference storage", symbol);
            }
        }
    }
    return true;
}

bool semantic_visible(SemanticProgram *program, const SemanticSymbol *symbol,
    const SemanticSymbol *initializing, SourceSpan site) {
    if (initializing == NULL) {
        return true;
    }
    if (initializing->role == SYMBOL_CONSTANT && symbol->role != SYMBOL_CONSTANT) {
        return semantic_fail(program, site, "A constant initializer can read only constants", symbol);
    }
    if (symbol->owner == initializing->owner && symbol->ordinal >= initializing->ordinal) {
        return semantic_fail(program, site, "The initializer source is not initialized yet", symbol);
    }
    return true;
}
