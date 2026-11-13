/** @file
 * @brief Validate exact storage identities independently of C names, sizes and offsets.
 * [COMP-02](../../../../spec/implementation/compiler.md#comp-02) retains canonical
 * semantic identities; [STATE-03](../../../../spec/language/declarations.md#state-03)
 * separates shared constants from writable instance fields.
 */
#include "internal.h"
bool plan_ir_type(const ProgramPlan *plan, const PlannedType *candidate) {
    for (const PlannedType *type = plan->first; type != NULL; type = type->next) {
        if (type == candidate) { return true; }
    }
    return false;
}
bool plan_ir_field(const PlannedType *type, const StorageField *candidate) {
    for (const StorageField *field = type->first; field != NULL; field = field->next) {
        if (field == candidate) { return true; }
    }
    return false;
}
bool plan_ir_storage(ProgramPlan *plan, const PlannedType *type) {
    const StorageField *field = type->first;
    const StorageField *previous = NULL;
    size_t slot = 0;
    for (const SemanticSymbol *symbol = type->type->first; symbol != NULL; symbol = symbol->next) {
        if (symbol->role == SYMBOL_CONSTANT) { continue; }
        StorageForm form = symbol->list ? (symbol->reference ? STORAGE_REFERENCE_LIST : STORAGE_OWNING_LIST)
            : symbol->reference ? STORAGE_REFERENCE : STORAGE_OWNED;
        if (field == NULL || field->symbol != symbol || field->previous != previous
            || field->slot != slot || field->form != form || !plan_ir_type(plan, field->type)
            || field->type->type != symbol->type) {
            return plan_fail(plan, symbol->site, "Typed IR storage does not match its resolved declaration");
        }
        previous = field;
        field = field->next;
        slot++;
    }
    if (field != NULL || previous != type->last || slot != type->field_count) {
        return plan_fail(plan, (SourceSpan){type->type->module->source->text, 0, 0},
            "Typed IR storage inventory is incomplete");
    }
    return true;
}
