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
