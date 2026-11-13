/** @file
 * @brief Validate exact lexical frame, chain and loop operands after control edge ownership is known.
 * [COMP-02](../../../../../spec/implementation/compiler.md#comp-02) forbids
 * unresolved source decisions; [CALL-09](../../../../../spec/language/calls-and-results.md#call-09)
 * requires an explicit initiating chain, never runtime lookup of a repeat/caller boundary.
 */
#include "internal.h"
/** Loop references are exact owner inventory identities, including null for non-loop instructions. */
static bool loop_member(const PlannedType *type, const PlannedLoop *candidate) {
    if (candidate == NULL) { return true; }
    for (const PlannedLoop *loop = type->loops; loop != NULL; loop = loop->next) {
        if (loop == candidate) { return true; }
    }
    return false;
}
/** Refusal abandons the whole pending tree before this link's own handler; an ancestor handler is never selected. */
static bool refusal(const PlannedType *type, const ControlNode *node) {
    if (node->operation != CONTROL_ACQUIRE_INSTANCE && node->operation != CONTROL_ACQUIRE_LIST
        && node->operation != CONTROL_SELECT_RESULT && node->operation != CONTROL_LIST_METHOD_PREFLIGHT) { return true; }
    if (node->chain == NULL || node->statement == NULL || node->statement->syntax->kind != AST_CALL) { return false; }
    const ControlNode *target = node->alternative;
    if (node->chain->header != NULL || node->chain->root->syntax->as.call.has_selected) {
        if (target == NULL || target->operation != CONTROL_ABANDON_CHAIN || target->chain != node->chain
            || target->statement != node->statement) { return false; }
        target = target->next;
    }
    if (!node->statement->syntax->as.call.has_alternative) { return target == type->failure; }
    return target != NULL && target->operation == CONTROL_REFUSAL_SNAPSHOT
        && target->statement == node->statement && target->chain == node->chain;
}
/** Loop operands follow the source owner; scope cleanup at return names the nearest enclosing traversal only. */
static const PlannedLoop *expected_loop(const PlannedType *type, const ControlNode *node) {
    if (node->statement == NULL) { return NULL; }
    if (node->operation == CONTROL_CLEANUP_SCOPES) {
        const SemanticStatement *scope = node->statement->enclosing_loop;
        while (scope != NULL && scope->syntax->kind != AST_FOREACH) { scope = scope->enclosing_loop; }
        return structured_loop(type, scope);
    }
    if (node->operation == CONTROL_CONDITION || node->operation == CONTROL_HEADER_ENTER
        || node->operation == CONTROL_RELEASE_CHAIN || node->operation == CONTROL_FOREACH_ENTER
        || node->operation == CONTROL_FOREACH_NEXT || node->operation == CONTROL_FOREACH_LEAVE
        || node->operation == CONTROL_FOREACH_EMPTY) { return structured_loop(type, node->statement); }
    return NULL;
}
bool plan_ir_structured(ProgramPlan *plan, const PlannedType *type) {
    if (!structured_loops(type) || !structured_chains(type)) {
        return plan_fail(plan, (SourceSpan){type->type->module->source->text, 0, 0},
            "Structured IR has incomplete lexical loop or initiating chain identities");
    }
    for (const ControlNode *node = type->controls; node != NULL; node = node->allocated_next) {
        const StorageFrame *frame = plan_storage_frame(type, node->statement);
        if (node->operation == CONTROL_CONDITION && node->statement != NULL
            && node->statement->condition_owner != node->statement) {
            frame = plan_storage_frame(type, node->statement->condition_owner);
        }
        if (node->frame != frame || !loop_member(type, node->loop) || node->loop != expected_loop(type, node)
            || (node->chain != NULL && !structured_chain(type, node->chain))) {
            return plan_fail(plan, node->site, "Structured instruction has a foreign frame, loop or initiating chain");
        }
        if (!refusal(type, node)) {
            return plan_fail(plan, node->site, "Refusal must abandon this initiating tree before its own link handler");
        }
        if (node->chain != NULL && node->statement != NULL && node->statement->syntax->kind == AST_CALL) {
            const SemanticStatement *root = node->statement;
            while (root->enclosing_operation != NULL) { root = root->enclosing_operation; }
            if (root != node->chain->root) {
                return plan_fail(plan, node->site, "Selected operation belongs to a different initiating caller chain");
            }
        }
    }
    return true;
}
