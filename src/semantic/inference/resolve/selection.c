/**
 * @file
 * @brief Resolve a direct named result through one typed parent and visit only its caller-side index names.
 * [CALL-09](../../../../spec/language/calls-and-results.md#call-09) forbids
 * fallback to caller/private names; [FLOW-04](../../../../spec/language/control-flow.md#flow-04)
 * uses the same selection boundary for its terminal owned condition.
 */
#include "../internal.h"
bool inference_select_result(Inference *inference, const InferenceOperation *parent,
    const AstAccess *target, InferenceValue *value) {
    if (parent->kind == INFER_LIST_METHOD) {
        if (!inference_result(inference, parent, target, 0, target->span, value)) { return false; }
    } else if (parent->kind == INFER_INSTANCE) {
        SemanticSymbol *field = semantic_interface_name(parent->value.type, SYMBOL_RESULT, target->name);
        if (field == NULL) { return semantic_fail(inference->program, target->name, "The operation has no such declared result",
            semantic_symbol(parent->value.type, target->name)); }
        *value = (InferenceValue){field->type, field, field->reference, field->list};
    } else {
        return semantic_fail(inference->program, target->span, "This service/namespace has no ordinary selected result", NULL);
    }
    if (target->reference != value->reference || (target->whole_list || target->index != NULL) != value->list) {
        return semantic_fail(inference->program, target->span, "Selected storage form differs from its declared result", value->symbol);
    }
    if (target->index != NULL) {
        InferenceValue index;
        if (!inference_access(inference, target->index, false, &index)) { return false; }
        if (index.reference || index.list || !index.type->abi.bindings[ABI_INDEX].present) {
            return semantic_fail(inference->program, target->index->span, "The index must be an owned scalar with the [] capability", index.symbol);
        }
    }
    value->list = target->whole_list;
    return true;
}
