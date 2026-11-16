/**
 * @file
 * @brief Compute greatest-fixed-point copy capability across all declared owned edges.
 *
 * [PROTO-07](../../../spec/implementation/native-interface.md#proto-07)
 * Start with native copy seeds and remove types reaching a noncopyable owned descendant, including empty-list declarations.
 */
#include "semantic/internal.h"

/** Compute the greatest closed subset of native-capable executable types.
 * Each removal points to an already removed prerequisite, so clone_cause chains
 * are finite witnesses ending at a native seed. References and constants never
 * participate; even an empty owning list contributes its element-type edge.
 * No allocation occurs. At most V removals and one final scan cost O(V(V+F)).
 * Component contract: [PROTO-07](../../../spec/implementation/native-interface.md#proto-07). */
void semantic_clone_capabilities(SemanticProgram *program) {
    for (SemanticType *type = program->first; type != NULL; type = type->next) {
        type->cloneable = type->executable && (type->module->syntax.sections.kind != MODULE_NATIVE
            || type->abi.bindings[ABI_CLONE].present);
        type->clone_cause = NULL;
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
