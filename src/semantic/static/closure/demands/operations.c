/** @file
 * @brief Require immediate source/list call Types and independently selected saved constant namespaces.
 * [CALL-09](../../../../../spec/language/calls-and-results.md#call-09) preserves selected-method provenance.
 * [CONST-03](../../../../../spec/language/declarations.md#const-03) fixes constant namespaces before reading members.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) retains actual required definitions.
 */
#include "internal.h"

/** Saved reads determine their namespace independently before result receiver
 * equations. A selected static export carries its already checked supplier.
 * [CONST-03](../../../../../spec/language/declarations.md#const-03) rejects receiver/member-driven inference.
 */
static bool read(StaticClosurePlan *plan, const StaticClosureEntry *entry, const AstNode *node, bool *ready) {
    StaticConnectionPlan *connections = plan->connections;
    const StaticConstantRead *source;
    StaticResultOutcome evaluated;
    if (!static_constant_read(connections->constants, entry->context, node, &source) ||
        !static_ordinary_evaluate(connections->constants->publications, source->source, &evaluated)) {
        (void)static_closure_dependencies(plan); return false;
    }
    const StaticResultContext *context;
    bool known;
    if (!static_closure_require(plan, entry->context, source->source->value, node->span, &context, &known)) { return false; }
    *ready &= known && evaluated == STATIC_RESULT_READY;
    if (!known) { return true; }
    bool changed;
    StaticResultOutcome outcome;
    if (!static_connection_read(connections, entry->context, node, context, &changed, &outcome)) {
        (void)static_closure_dependencies(plan); return false;
    }
    *ready &= outcome == STATIC_RESULT_READY;
    return true;
}

/** Source calls require their actual nominal target; intrinsic list interfaces
 * require their element and fixed position/result integer Types without creating
 * fake method instances or source fields.
 * [LIST-04](../../../../../spec/language/lists.md#list-04) fixes intrinsic ports.
 */
static bool call(StaticClosurePlan *plan, const StaticClosureEntry *entry, const AstNode *node, bool *ready) {
    const StaticTypedOperation *operation;
    StaticResultOutcome outcome;
    if (!static_connection_operation(plan->connections, entry->context, node, &operation, &outcome)) {
        (void)static_closure_dependencies(plan); return false;
    }
    if (outcome == STATIC_RESULT_PENDING) { *ready = false; return true; }
    const StaticTerm *types[2] = {operation->target ? operation->target->owner : operation->element,
        operation->kind == STATIC_OPERATION_SOURCE ? NULL : operation->integer};
    for (size_t index = 0; index < 2; index++) {
        if (!types[index]) { continue; }
        const StaticResultContext *context;
        bool known;
        if (!static_closure_require(plan, entry->context, types[index], node->span, &context, &known)) { return false; }
        *ready &= known;
    }
    return true;
}

/** Original source events include nested/refusal operations exactly once;
 * unresolved targets never suppress independent later demands.
 * [TPL-06](../../../../../spec/language/templates.md#tpl-06) checks skipped runtime branches.
 */
bool static_closure_operations(StaticClosurePlan *plan, const StaticClosureEntry *entry, bool *output) {
    bool ready = true;
    for (const StaticRuntimeEvent *event = entry->scope->runtime->first_event; event; event = event->next) {
        if (event->kind != STATIC_RUNTIME_CALL) { continue; }
        bool success = event->node->as.call.mode == AST_CONSTANT_READ ?
            read(plan, entry, event->node, &ready) : call(plan, entry, event->node, &ready);
        if (!success) { return false; }
    }
    *output = ready;
    return true;
}
