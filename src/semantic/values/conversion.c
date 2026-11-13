/** @file
 * @brief Require an owned scalar's own native condition/index conversion without implicit reference dereference or field inheritance.
 * [FLOW-01](../../../spec/language/control-flow.md#flow-01) and
 * [LIST-02](../../../spec/language/lists.md#list-02) apply the same predicate
 * to real declared accesses and explicitly typed ephemeral intrinsic values.
 */
#include "semantic/internal.h"
bool semantic_conversion(SemanticProgram *program, SemanticType *type, bool reference,
    bool list, AbiOperation capability, SourceSpan site, const SemanticSymbol *declaration) {
    if (reference || list || !type->abi.bindings[capability].present) {
        semantic_fail(program, site, capability == ABI_INDEX
            ? "The index must be an owned scalar with the [] capability"
            : "The condition must be an owned scalar with the ? capability", declaration);
        program->error.subject = type->module->source->logical_path;
        return false;
    }
    return true;
}
