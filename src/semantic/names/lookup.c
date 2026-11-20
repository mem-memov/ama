/**
 * @file
 * @brief Resolve complete accepted declaration names without normalization or path-based Type guessing.
 *
 * [NAME-04](../../../spec/language/declarations.md#name-04)
 * [TPL-05](../../../spec/language/templates.md#tpl-05) makes a bare source path
 * insufficient when distinct required capture maps share one module origin.
 * Declaration lookup consumes projected source-bound fields; full Type contexts remain separate from names.
 * [TYPE-06](../../../spec/language/declarations.md#type-06) Inference introduces exact semantic names without manufacturing declarations.
 */
#include "semantic/internal.h"

SemanticSymbol *semantic_symbol(const SemanticType *type, SourceSpan name) {
    for (SemanticSymbol *symbol = type->first; symbol != NULL; symbol = symbol->next) {
        if (semantic_same_text(symbol->name, name)) {
            return symbol;
        }
    }
    return NULL;
}
