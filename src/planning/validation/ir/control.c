/** @file
 * @brief Check control graph ownership, unique selectors and resolved operands before publication.
 * [COMP-02](../../../../spec/implementation/compiler.md#comp-02) retains explicit
 * failure and continuation edges; [GEN-04](../../../../spec/implementation/compiler.md#gen-04)
 * assigns selectors independently of target C labels. Runtime cycles are not traversed.
 */
#include "internal.h"
/** Compare an edge with owner inventory addresses before any dereference. */
static bool member(const PlannedType *type, const ControlNode *candidate) {
    for (const ControlNode *node = type->controls; node != NULL; node = node->allocated_next) {
        if (node == candidate) { return true; }
    }
    return false;
}
/** A statement must be the exact fully resolved record owned by this nominal module. */
static bool statement_member(const PlannedType *type, const SemanticStatement *candidate) {
    for (const SemanticStatement *statement = type->type->statements; statement != NULL; statement = statement->next) {
        if (statement == candidate) { return true; }
    }
    return false;
}
/** Supplied binding identity must belong to this node's resolved statement. */
static bool binding_member(const SemanticStatement *statement, const SemanticBinding *candidate) {
    if (statement == NULL) { return false; }
    for (const SemanticBinding *binding = statement->bindings; binding != NULL; binding = binding->next) {
        if (binding == candidate) { return true; }
    }
    return false;
}
/** Compare every assigned selector exactly; neither hash equality nor reachability determines identity. */
static bool unique(const ProgramPlan *plan, const ControlNode *candidate) {
    if (candidate->selector == 0 || candidate->selector >= plan->next_selector) { return false; }
    for (const PlannedType *type = plan->first; type != NULL; type = type->next) {
        for (const ControlNode *node = type->controls; node != NULL; node = node->allocated_next) {
            if (node != candidate && node->selector == candidate->selector) { return false; }
        }
    }
    return true;
}
/** Only entry transfers cross nominal graph ownership; resumes use dynamic saved positions. */
static bool edges(const ProgramPlan *plan, const PlannedType *type, const ControlNode *node) {
    if ((node->alternative != NULL && !member(type, node->alternative))
        || (node->failure != NULL && !member(type, node->failure))
        || (node->resume != NULL && !member(type, node->resume))) { return false; }
    if (node->operation == CONTROL_ENTER_INSTANCE) {
        return plan_ir_type(plan, node->callee) && node->next == node->callee->entry;
    }
    return node->next == NULL || member(type, node->next);
}
/** Installation operands are exact callee declarations, even when a default has no supplied binding. */
static bool installed_field(const ControlNode *node) {
    if (node->statement == NULL || node->statement->namespace_type == NULL) { return false; }
    SymbolRole role = node->operation == CONTROL_INSTALL_PARAMETER ? SYMBOL_PARAMETER : SYMBOL_RESULT;
    for (const SemanticSymbol *symbol = node->statement->namespace_type->first; symbol != NULL; symbol = symbol->next) {
        if (symbol != node->field) { continue; }
        StorageForm form = symbol->list ? (symbol->reference ? STORAGE_REFERENCE_LIST : STORAGE_OWNING_LIST)
            : symbol->reference ? STORAGE_REFERENCE : STORAGE_OWNED;
        return symbol->role == role && symbol->role_ordinal == node->position
            && symbol->type == node->value_type && form == node->value_form;
    }
    return false;
}
/** Validate operation-specific mandatory operands, keeping refusal separate from technical failure. */
static bool operands(const ProgramPlan *plan, const PlannedType *type, const ControlNode *node) {
    bool terminal = node->operation == CONTROL_COMPLETE || node->operation == CONTROL_YIELD
        || node->operation == CONTROL_FAILURE || node->operation == CONTROL_RESUME_INSTANCE;
    if (!terminal && node->next == NULL) { return false; }
    if (node->operation == CONTROL_CONDITION || node->operation == CONTROL_ACQUIRE_INSTANCE) {
        if (node->alternative == NULL) { return false; }
    }
    if (node->operation == CONTROL_SAVE_POSITION || node->operation == CONTROL_SET_RETURN) {
        if (node->resume == NULL) { return false; }
    }
    if (node->operation == CONTROL_SET_RETURN || node->operation == CONTROL_RESUME_INSTANCE
        || node->operation == CONTROL_ENTER_INSTANCE) {
        if (!plan_ir_type(plan, node->callee) || node->statement == NULL
            || node->callee->type != node->statement->namespace_type) { return false; }
    }
    if (node->binding != NULL) {
        const SemanticBinding *binding = node->binding;
        StorageForm form = binding->list ? (binding->reference ? STORAGE_REFERENCE_LIST : STORAGE_OWNING_LIST)
            : binding->reference ? STORAGE_REFERENCE : STORAGE_OWNED;
        if (node->field != binding->field || node->value_type != binding->type
            || node->position != binding->position || node->value_form != form) { return false; }
    }
    if (node->operation >= CONTROL_SELECT_RECEIVER && node->operation != CONTROL_NATIVE_ENTRY) {
        if (node->statement == NULL || node->failure != type->failure) { return false; }
    }
    if (node->operation == CONTROL_EVALUATE_INPUT || node->operation == CONTROL_TRANSFER_OUTPUT) {
        if (node->binding == NULL || node->binding->type == NULL) { return false; }
    }
    if (node->operation == CONTROL_INSTALL_PARAMETER || node->operation == CONTROL_INITIALIZE_RESULT) {
        if (!installed_field(node)) { return false; }
    }
    if (node->operation == CONTROL_CONDITION) {
        if (node->statement == NULL || node->statement->condition.type == NULL
            || node->statement->condition.reference || node->statement->condition.list) { return false; }
    }
    return true;
}
bool plan_ir_control(ProgramPlan *plan, const PlannedType *type) {
    if (type->requires_pool && (!member(type, type->entry) || !member(type, type->complete)
            || !member(type, type->yield) || !member(type, type->failure))) {
        return plan_fail(plan, (SourceSpan){type->type->module->source->text, 0, 0},
            "Typed IR executable type has an unresolved endpoint");
    }
    for (const ControlNode *node = type->controls; node != NULL; node = node->allocated_next) {
        if (node->type != type || node->operation < CONTROL_COMPLETE || node->operation > CONTROL_RELEASE_TARGET
            || !source_span_valid(node->site) || node->site.source != type->type->module->source->text
            || !unique(plan, node) || !edges(plan, type, node)
            || (node->statement != NULL && !statement_member(type, node->statement))
            || (node->binding != NULL && !binding_member(node->statement, node->binding))
            || !operands(plan, type, node)) {
            return plan_fail(plan, node->site, "Typed IR control instruction has an unresolved operand, selector or edge");
        }
    }
    return true;
}
