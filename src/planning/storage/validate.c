/** @file
 * @brief Check complete logical storage without requiring lowered control or generated C.
 * [RT-02](../../../spec/implementation/runtime.md#rt-02) keeps general stable
 * identity/control state explicit; [COMP-03](../../../spec/implementation/compiler.md#comp-03)
 * orders layout through eager ownership only, never runtime invocation edges.
 */
#include "planning/validation/ir/internal.h"
bool plan_validate_storage(ProgramPlan *plan) {
    if (!plan_ir_type(plan, plan->entry) || plan->entry->type->module != plan->semantic->project->entry) {
        return plan_fail(plan, (SourceSpan){0}, "Storage has no exact executable entry identity");
    }
    size_t pools = 0;
    size_t constants = 0;
    const SemanticType *semantic = plan->semantic->first;
    for (const PlannedType *type = plan->first; type != NULL; type = type->next) {
        if (semantic == NULL || type->type != semantic) {
            return plan_fail(plan, (SourceSpan){0}, "Storage requires the complete exact nominal inventory");
        }
        unsigned service = semantic->executable ? STORAGE_GENERATION | STORAGE_BUSY | STORAGE_POSITION
            | STORAGE_RETURN_LINK | STORAGE_REFUSAL | STORAGE_ACQUISITION
            | STORAGE_CONSTRUCTING_IDENTITY | STORAGE_OWNERSHIP : 0;
        if (type->requires_pool != semantic->executable || type->stable_slots != semantic->executable
            || type->service_storage != service || !plan_ir_storage(plan, type) || !plan_ir_frames(plan, type)) {
            if (plan->status == PLAN_OK) {
                plan_fail(plan, (SourceSpan){semantic->module->source->text, 0, 0}, "Storage requires the general stable-slot logical state");
            }
            return false;
        }
        pools += type->requires_pool;
        semantic = semantic->next;
    }
    if (semantic != NULL) { return plan_fail(plan, (SourceSpan){0}, "Storage is missing a resolved nominal type"); }
    size_t ordinal = 0;
    for (const GraphVertex *vertex = plan->semantic->eager.order; vertex != NULL; vertex = vertex->order_next) {
        const PlannedType *type = program_plan_type(plan, vertex->value);
        if (type == NULL || type->layout_ordinal != ordinal++) {
            return plan_fail(plan, (SourceSpan){0}, "Storage layout order differs from exact eager dependencies");
        }
    }
    for (const SemanticConstantPlan *value = plan->semantic->constant_first; value != NULL; value = value->next) { constants++; }
    unsigned domains = CONTEXT_POOL_DIRECTORY | CONTEXT_SHARED_VALUES | CONTEXT_CURRENT_INSTANCE
        | CONTEXT_EXECUTOR | CONTEXT_ROOT_LEDGER | CONTEXT_DIAGNOSTICS;
    if (plan->context_storage.domains != domains || plan->context_storage.pools != pools
        || plan->context_storage.constants != constants) {
        return plan_fail(plan, (SourceSpan){0}, "Context storage domains or exact inventory counts are incomplete");
    }
    const SemanticConstantPlan *constant = plan->semantic->constant_first;
    const SharedStorage *previous = NULL;
    for (const SharedStorage *storage = plan->shared_first; storage != NULL; storage = storage->next) {
        if (constant == NULL || storage->constant != constant || storage->previous != previous
            || !storage->immutable || storage->service_storage != 0
            || !plan_ir_type(plan, storage->value_type) || storage->value_type->type != constant->symbol->type
            || storage->form != (constant->symbol->list ? STORAGE_OWNING_LIST : STORAGE_OWNED)) {
            return plan_fail(plan, (SourceSpan){0}, "Context shared storage differs from resolved constant identities");
        }
        previous = storage;
        constant = constant->next;
    }
    if (constant != NULL || previous != plan->shared_last) {
        return plan_fail(plan, (SourceSpan){0}, "Context shared storage inventory is incomplete");
    }
    return true;
}
