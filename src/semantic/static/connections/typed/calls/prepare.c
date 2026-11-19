/** @file
 * @brief Retain proven ordinary call interfaces without certifying callee expansion.
 * [TPL-10](../../../../../../spec/language/templates.md#tpl-10) uses only independently proven nominal origins.
 * [CALL-08](../../../../../../spec/language/calls-and-results.md#call-08) fixes named/positional interface bindings.
 * [TPL-11](../../../../../../spec/language/templates.md#tpl-11) separately guards actual callee creation.
 */
#include <stdint.h>
#include "internal.h"
#include "../../../bindings/normalize.h"

/** Original event membership rejects same-shaped foreign syntax before use.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) retains exact original operation identity.
 */
static bool has_operation(const StaticTypedScope *scope, const AstNode *node) {
    for (const StaticRuntimeEvent *event = scope->runtime->first_event; event; event = event->next) {
        if (event->kind == STATIC_RUNTIME_CALL && event->node == node) { return true; }
    }
    return false;
}

/** Resolve one scalar nominal head with possibly unknown captures, then snapshot
 * its original ordinary interface. This is metadata, not a real callee context;
 * no body equations or nested execution are triggered while preparing it.
 * [TPL-10](../../../../../../spec/language/templates.md#tpl-10) preserves finite interface facts.
 */
static bool prepare(StaticConnectionPlan *plan, const StaticTypedScope *caller,
    const AstNode *node, const StaticTypedOperation **output, StaticResultOutcome *outcome) {
    StaticTypedOperand target;
    StaticResultOutcome selected;
    if (!static_typed_target(plan, caller, node, &target, &selected)) { return false; }
    if (selected == STATIC_RESULT_PENDING) { *outcome = STATIC_RESULT_PENDING; return true; }
    if (target.list) {
        if (!static_typed_list_service(plan, caller, node, &target, output)) { return false; }
        *outcome = STATIC_RESULT_READY;
        return true;
    }
    StaticApplicationPlan *applications = plan->constants->publications->results->guards->applications;
    StaticConstraintSolver *solver = applications->solver;
    const StaticTerm *nominal;
    StaticConstraintOutcome known;
    if (!static_constraint_nominal(solver, target.type, &nominal, &known)) {
        (void)static_connection_dependencies(plan); return false;
    }
    if (known == STATIC_CONSTRAINT_PENDING) { *outcome = STATIC_RESULT_PENDING; return true; }
    if (nominal->category != STATIC_CATEGORY_TYPE) {
        return static_connection_fail(plan, STATIC_CONNECTION_CONFLICT, node->span, "Operation target requires an ordinary module Type");
    }
    const StaticTermEnvironment *environment;
    if (!static_term_environment(solver->terms, nominal->as.nominal.origin, nominal, &environment)) {
        (void)static_connection_dependencies(plan); return false;
    }
    const StaticTypedScope *scope;
    bool changed;
    if (!static_connection_scope(plan, environment, &scope) || !static_connection_annotations(plan, scope, &changed)) { return false; }
    BindingInterface *view;
    BindingApplication *bindings;
    if (!binding_source_interface(applications->bindings, nominal->as.nominal.origin, BINDING_MODULE_SURFACE, NULL, 0, &view) ||
        !binding_normalize_runtime(applications->bindings, view, node, &bindings)) {
        (void)static_connection_dependencies(plan); return false;
    }
    if (plan->operation_count == SIZE_MAX) {
        return static_connection_fail(plan, STATIC_CONNECTION_OUT_OF_MEMORY, node->span, "Ordinary operation metadata count exceeds representable storage");
    }
    StaticTypedOperation *operation = static_connection_allocate(plan, sizeof(*operation));
    if (!operation) { return false; }
    *operation = (StaticTypedOperation){.next = plan->first_operation, .caller = caller, .syntax = node,
        .target = scope, .bindings = bindings};
    plan->first_operation = operation;
    plan->operation_count++;
    *output = operation;
    *outcome = STATIC_RESULT_READY;
    return true;
}

/** Cache only successfully proven original interface records. Unknown origins
 * remain retryable without inventing an interface, nominal key or callee root.
 * Selected operations require their parent's separately prepared interface.
 * [CALL-09](../../../../../../spec/language/calls-and-results.md#call-09) preserves parent result provenance.
 */
bool static_connection_operation(StaticConnectionPlan *plan, const StaticResultContext *actual,
    const AstNode *node, const StaticTypedOperation **output, StaticResultOutcome *outcome) {
    if (!static_connection_dependencies(plan)) { return false; }
    if (!output || !outcome) {
        return static_connection_fail(plan, STATIC_CONNECTION_INVALID, (SourceSpan){0}, "Missing ordinary operation metadata or outcome receiver");
    }
    const StaticTypedScope *caller;
    if (!static_connection_context(plan, actual, &caller)) { return false; }
    if (!has_operation(caller, node)) {
        return static_connection_fail(plan, STATIC_CONNECTION_INVALID, (SourceSpan){0}, "Operation syntax is not an original caller event");
    }
    for (const StaticTypedOperation *item = plan->first_operation; item; item = item->next) {
        if (item->caller == caller && item->syntax == node) { *output = item; *outcome = STATIC_RESULT_READY; return true; }
    }
    for (const StaticTypedOperation *parent = plan->first_operation; parent; parent = parent->next) {
        if (parent->caller == caller && parent->syntax == node->as.call.enclosing_operation && parent->kind == STATIC_OPERATION_LIST_SERVICE) {
            if (!static_typed_list_method(plan, parent, node, output)) { return false; }
            *outcome = STATIC_RESULT_READY;
            return true;
        }
    }
    if (node->as.call.mode == AST_CONSTANT_READ) { *outcome = STATIC_RESULT_PENDING; return true; }
    return prepare(plan, caller, node, output, outcome);
}
