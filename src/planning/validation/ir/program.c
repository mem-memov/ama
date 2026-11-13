/** @file
 * @brief Validate a complete typed plan before publishing it to C emission.
 * [COMP-02](../../../../spec/implementation/compiler.md#comp-02) forbids unresolved
 * source decisions at this boundary. Exact owner inventories permit checking cyclic
 * executable edges without running code, reparsing source or allocating scratch memory.
 */
#include "internal.h"
bool program_plan_validate(ProgramPlan *plan) {
    if (!plan->attempted || plan->status != PLAN_OK) { return false; }
    bool published = plan->valid;
    plan->valid = false;
    if (!plan->storage_ready || !plan->semantic->constants_valid || !plan->semantic->resolved || !plan_ir_type(plan, plan->entry)) {
        return plan_fail(plan, (SourceSpan){0}, "Typed IR requires complete semantic analysis and an exact entry identity");
    }
    if (!plan_validate_storage(plan)) { return false; }
    const SemanticType *semantic = plan->semantic->first;
    for (const PlannedType *type = plan->first; type != NULL; type = type->next) {
        if (semantic == NULL || type->type != semantic || type->requires_pool != semantic->executable) {
            return plan_fail(plan, (SourceSpan){0}, "Typed IR nominal inventory differs from resolved types");
        }
        if (!plan_ir_lifecycle(plan, type) || !plan_ir_control(plan, type) || !plan_ir_structured(plan, type) || !plan_ir_replacements(plan, type) || !plan_ir_invocations(plan, type)) {
            return false;
        }
        semantic = semantic->next;
    }
    if (semantic != NULL) { return plan_fail(plan, (SourceSpan){0}, "Typed IR is missing a resolved nominal type"); }
    const SemanticConstantPlan *constant = plan->semantic->constant_first;
    const SharedStorage *previous = NULL;
    for (const SharedStorage *storage = plan->shared_first; storage != NULL; storage = storage->next) {
        if (constant == NULL || storage->constant != constant || storage->previous != previous
            || !storage->immutable || storage->service_storage != 0
            || !plan_ir_type(plan, storage->value_type) || storage->value_type->type != constant->symbol->type
            || storage->form != (constant->symbol->list ? STORAGE_OWNING_LIST : STORAGE_OWNED)) {
            return plan_fail(plan, (SourceSpan){0}, "Typed IR shared storage differs from resolved initialization order");
        }
        previous = storage;
        constant = constant->next;
    }
    if (constant != NULL || previous != plan->shared_last
        || plan->constant_startup != plan->semantic->constant_first
        || plan->constant_cleanup != plan->semantic->constant_last) {
        return plan_fail(plan, (SourceSpan){0}, "Typed IR shared initialization endpoints are incomplete");
    }
    plan->valid = published;
    return true;
}
