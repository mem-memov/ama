/** @file
 * @brief Consume original runtime events once to fix lexical identities independently of Type values.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) fixes receiver declarations from source order.
 * [CALL-09](../../../../../../spec/language/calls-and-results.md#call-09) keeps selector bases foreign and indices caller-scoped.
 */
#include "../../internal.h"

/** Foreign labels/selectors contribute only their caller-side index operands.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) excludes their base names from encounters.
 */
static bool indices(StaticConnectionPlan *plan, StaticRuntimeModule *module,
    const StaticRuntimeEvent *event, const AstAccess *selector) {
    if (selector == NULL || selector->index == NULL) { return true; }
    const StaticRuntimeField *field;
    return static_runtime_access(plan, module, event, selector->index, true, &field);
}

/** Root targets bind caller storage; selected targets use their parent's declared
 * interface later, so only their indices are caller occurrences. Namespace Type
 * syntax remains in the static namespace, independently of result receiver names.
 * [CALL-09](../../../../../../spec/language/calls-and-results.md#call-09) preserves lexical caller context.
 */
static bool call(StaticConnectionPlan *plan, StaticRuntimeModule *module, const StaticRuntimeEvent *event) {
    const AstNode *node = event->node;
    if (node->as.call.mode == AST_CONSTANT_READ) { return true; }
    if (node->as.call.enclosing_operation) { return indices(plan, module, event, node->as.call.target); }
    const StaticRuntimeField *field;
    if (!static_runtime_access(plan, module, event, node->as.call.target, false, &field)) { return false; }
    if (field->role == STATIC_FIELD_CONSTANT) {
        return static_connection_fail(plan, STATIC_CONNECTION_CONFLICT, event->site, "Shared constants cannot be operation targets");
    }
    return true;
}

/** Bind inputs only as caller sources, and outputs only after their label indices.
 * Result/constant/member Type equations are separate from these name decisions.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) prevents output inference from importing callee names.
 */
static bool binding(StaticConnectionPlan *plan, StaticRuntimeModule *module, const StaticRuntimeEvent *event) {
    const AstArgument *argument = event->argument;
    if (argument->omitted) { return true; }
    if (argument->kind == AST_INPUT) { return static_runtime_source(plan, module, event, argument->source); }
    const StaticRuntimeReceiver *receiver;
    return indices(plan, module, event, argument->label) &&
        static_runtime_receiver(plan, module, event, argument->receiver, &receiver);
}

/** A fresh assignment receiver is visible only after rejecting self-source in
 * this introducing transfer. All later events retain the same original field.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) distinguishes lexical and equality cycles.
 */
static bool assignment(StaticConnectionPlan *plan, StaticRuntimeModule *module, const StaticRuntimeEvent *event) {
    const StaticRuntimeReceiver *receiver;
    if (!static_runtime_receiver(plan, module, event, event->node->as.assignment.receiver, &receiver)) { return false; }
    module->inferring = receiver->introduced ? receiver->field->name : (SourceSpan){0};
    if (!static_runtime_source(plan, module, event, event->node->as.assignment.source)) { return false; }
    module->inferring = (SourceSpan){0};
    return true;
}

/** Every sorted event is consumed once; Type refinement cannot rerun this pass
 * or choose different bindings. Locals are selected by original body intervals.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) fixes the encountered prefix invariant.
 */
bool static_runtime_bind(StaticConnectionPlan *plan, StaticRuntimeModule *module) {
    for (const StaticRuntimeEvent *event = module->first_event; event; event = event->next) {
        switch (event->kind) {
            case STATIC_RUNTIME_DECLARATION:
                if (!static_runtime_encounter(plan, module, event->node->as.declaration.name->name, NULL)) { return false; }
                break;
            case STATIC_RUNTIME_INITIALIZER:
                if (!static_runtime_source(plan, module, event, event->source)) { return false; }
                break;
            case STATIC_RUNTIME_ASSIGNMENT:
                if (!assignment(plan, module, event)) { return false; }
                break;
            case STATIC_RUNTIME_CALL:
                if (!call(plan, module, event)) { return false; }
                break;
            case STATIC_RUNTIME_BINDING:
                if (!binding(plan, module, event)) { return false; }
                break;
            case STATIC_RUNTIME_CONDITION: {
                const AstNode *node = event->node;
                if (node->as.control.condition_owner) {
                    if (!indices(plan, module, event, node->as.control.condition)) { return false; }
                } else {
                    const StaticRuntimeField *field;
                    if (!static_runtime_access(plan, module, event, node->as.control.condition, false, &field)) { return false; }
                }
                break;
            }
            case STATIC_RUNTIME_FOREACH:
                if (!static_runtime_foreach(plan, module, event)) { return false; }
                break;
        }
    }
    return true;
}
