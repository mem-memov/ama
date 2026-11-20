/**
 * @file
 * @brief Query a uniquely identified source origin and validate typed initializer visibility.
 *
 * [TYPE-01](../../../spec/language/declarations.md#type-01)
 * [TPL-05](../../../spec/language/templates.md#tpl-05) requires unique source-path
 * lookups to refuse ambiguity instead of selecting an arbitrary capture map.
 * Full-key field Types are supplied by guarded projection; these queries neither link nor infer a declaration.
 * [TYPE-06](../../../spec/language/declarations.md#type-06) Inferred types already name exact known identities and need no invented type-path line.
 */
#include <string.h>
#include "semantic/internal.h"

SemanticType *semantic_find_type(const SemanticProgram *program, const char *logical_path) {
    SemanticType *selected = NULL;
    if (logical_path != NULL) {
        for (SemanticType *type = program->first; type != NULL; type = type->next) {
            if (strcmp(type->module->source->logical_path, logical_path) == 0) {
                if (selected) { return NULL; }
                selected = type;
            }
        }
    }
    return selected;
}

bool semantic_visible(SemanticProgram *program, const SemanticSymbol *symbol,
    const SemanticSymbol *initializing, SourceSpan site) {
    SemanticVisibility visibility = semantic_initializer_visibility(symbol, initializing);
    if (visibility == SEMANTIC_CONSTANT_ONLY) {
        return semantic_fail(program, site, "A constant initializer can read only constants", symbol);
    }
    if (visibility == SEMANTIC_NOT_INITIALIZED) {
        return semantic_fail(program, site, "The initializer source is not initialized yet", symbol);
    }
    return true;
}
