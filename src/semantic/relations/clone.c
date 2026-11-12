/**
 * @file
 * @brief Compute greatest-fixed-point copy capability across all declared owned edges.
 *
 * [PROTO-07](../../../spec/implementation/native-interface.md#proto-07)
 * Start with native copy seeds and remove types reaching a noncopyable owned descendant, including empty-list declarations.
 */
#include "semantic/internal.h"

void semantic_clone_capabilities(SemanticProgram *program) {
    for (SemanticType *type = program->first; type != NULL; type = type->next) {
        type->cloneable = type->executable && (type->module->syntax.sections.kind != MODULE_NATIVE
            || type->abi.bindings[ABI_CLONE].present);
        type->clone_seed = !type->cloneable && type->executable ? type : NULL;
    }
    bool changed;
    do {
        changed = false;
        for (SemanticType *type = program->first; type != NULL; type = type->next) {
            if (!type->cloneable) {
                continue;
            }
            for (SemanticSymbol *symbol = type->first; symbol != NULL; symbol = symbol->next) {
                if (symbol->role != SYMBOL_CONSTANT && !symbol->reference && !symbol->type->cloneable) {
                    type->cloneable = false;
                    type->clone_seed = symbol->type->clone_seed;
                    type->clone_cause = symbol;
                    changed = true;
                    break;
                }
            }
        }
    } while (changed);
}
