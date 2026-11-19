/** @file
 * @brief Relate original caller operands and proven callee interface field Types.
 * [TPL-03](../../../../../../spec/language/templates.md#tpl-03) derives Type bindings from ordinary connections.
 * [CALL-08](../../../../../../spec/language/calls-and-results.md#call-08) preserves exact input/result storage forms.
 */
#include "internal.h"

/** Frozen interface slots identify original declarations, never same-named
 * private fields or fields of another independently instantiated callee.
 * [CALL-08](../../../../../../spec/language/calls-and-results.md#call-08) fixes the actual interface slot.
 */
static const StaticTypedField *interface_field(const StaticTypedOperation *operation, const BindingSlot *slot) {
    for (const StaticTypedField *field = operation->target->first_field; field; field = field->next) {
        if (field->definition->statement == slot->runtime_declaration) { return field; }
    }
    return NULL;
}

/** Original binding events supply exact caller source/receiver provenance.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) prohibits name rebinding after inference.
 */
static const StaticRuntimeEvent *event_for(const StaticTypedOperation *operation, const AstArgument *argument) {
    for (const StaticRuntimeEvent *event = operation->caller->runtime->first_event; event; event = event->next) {
        if (event->kind == STATIC_RUNTIME_BINDING && event->node == operation->syntax && event->argument == argument) { return event; }
    }
    return NULL;
}

/** Each non-skipped original request contributes one symmetric Type equality.
 * Literal capability and qualified namespace work remain explicit obligations;
 * skipped slots consume positions without introducing values or Type anchors.
 * [TPL-03](../../../../../../spec/language/templates.md#tpl-03) permits inference through both inputs and outputs.
 */
static bool request(StaticConnectionPlan *plan, const StaticResultContext *actual, const StaticTypedOperation *operation,
    const BindingRequest *binding, bool *ready) {
    if (binding->omitted) { return true; }
    const AstArgument *argument = binding->syntax.runtime_binding;
    const StaticTypedField *field = operation->kind == STATIC_OPERATION_SOURCE ? interface_field(operation, binding->slot) : NULL;
    const StaticTerm *type = field ? field->type : NULL;
    if (operation->kind == STATIC_OPERATION_LIST_METHOD && binding->slot->intrinsic_field) {
        type = binding->slot->intrinsic_field->element_reference ? operation->element : operation->integer;
    }
    const StaticRuntimeEvent *event = event_for(operation, argument);
    if (!type || !event) {
        return static_connection_fail(plan, STATIC_CONNECTION_INVALID, binding->span, "Binding lacks its original interface field or caller event");
    }
    StaticTypedOperand operand;
    if (argument->kind == AST_INPUT) {
        StaticResultOutcome outcome;
        if (!static_typed_operand(plan, operation->caller, event, argument->source, &operand, &outcome)) { return false; }
        if (outcome == STATIC_RESULT_PENDING) {
            if (argument->source->kind == AST_LITERAL_SOURCE && (binding->slot->reference || binding->slot->list)) {
                return static_connection_fail(plan, STATIC_CONNECTION_CONFLICT, argument->span, "Input literal requires owned scalar storage");
            }
            bool resolved;
            if (!static_typed_defer_constant(plan, actual, argument->source, type, binding->slot->reference, binding->slot->list, &resolved)) { return false; }
            *ready &= resolved;
            return true;
        }
    } else if (!static_typed_receiver_operand(plan, operation->caller, operation->syntax, argument->receiver, &operand)) { return false; }
    if (operand.reference != binding->slot->reference || operand.list != binding->slot->list) {
        return static_connection_fail(plan, STATIC_CONNECTION_CONFLICT, argument->span, "Binding operand storage form differs from its interface field");
    }
    StaticConstraintSolver *solver = plan->constants->publications->results->guards->applications->solver;
    if (!static_constraint_equate(solver, type, operand.type, argument->span)) {
        (void)static_connection_dependencies(plan); return false;
    }
    return true;
}

/** Known interfaces may constrain unknown captures without expanding the callee
 * body. Later genuine equations can make an unknown target interface available;
 * no name/signature heuristic or fabricated default instance supplies it.
 * [TPL-10](../../../../../../spec/language/templates.md#tpl-10) preserves exact refinement.
 */
bool static_connection_bindings(StaticConnectionPlan *plan, const StaticResultContext *actual,
    const AstNode *node, bool *changed, StaticResultOutcome *outcome) {
    if (!static_connection_dependencies(plan)) { return false; }
    if (!changed || !outcome) {
        return static_connection_fail(plan, STATIC_CONNECTION_INVALID, (SourceSpan){0}, "Missing ordinary binding progress or outcome receiver");
    }
    StaticConstraintSolver *solver = plan->constants->publications->results->guards->applications->solver;
    size_t before = solver->equation_count;
    const StaticTypedOperation *operation;
    StaticResultOutcome known;
    if (!static_connection_operation(plan, actual, node, &operation, &known)) { return false; }
    bool ready = known == STATIC_RESULT_READY;
    if (ready) {
        for (const BindingRequest *binding = operation->bindings ? operation->bindings->first : NULL; binding; binding = binding->next) {
            if (!request(plan, actual, operation, binding, &ready)) { return false; }
        }
    }
    *changed = solver->equation_count != before;
    *outcome = ready ? STATIC_RESULT_READY : STATIC_RESULT_PENDING;
    return true;
}
