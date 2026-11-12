/**
 * @file
 * @brief Resolve declaration initializers and select one compatible construction path.
 *
 * [TYPE-02](../../../spec/language/declarations.md#type-02)
 * Each list initializer is a scalar element; native literal/copy capability is checked before planning.
 */
#include "semantic/internal.h"

bool semantic_initializers(SemanticProgram *program, SemanticType *type) {
    for (SemanticSymbol *symbol = type->first; symbol != NULL; symbol = symbol->next) {
        if (symbol->role == SYMBOL_CONSTANT && !symbol->type->cloneable) {
            return semantic_fail(program, symbol->declaration->as.declaration.type,
                "A shared constant requires an independently copyable type", symbol);
        }
        for (AstInitializer *value = symbol->declaration->as.declaration.first;
            value != NULL; value = value->next) {
            SemanticInitializer *initializer = semantic_allocate(program, sizeof(*initializer));
            if (initializer == NULL) {
                return false;
            }
            initializer->source = semantic_source(program, type, value->source, symbol);
            if (initializer->source == NULL || !semantic_transfer(program, symbol->type,
                    symbol->reference, false, initializer->source, NULL, symbol, &initializer->transfer)) {
                return false;
            }
            if (symbol->last_initializer == NULL) {
                symbol->initializers = initializer;
            } else {
                symbol->last_initializer->next = initializer;
            }
            symbol->last_initializer = initializer;
        }
    }
    return true;
}
