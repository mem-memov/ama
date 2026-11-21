/** @file
 * @brief Read a declaration's exact foreign result without inferring supplier inputs from its consumer.
 * [TPL-15](../../../../../spec/language/templates.md#tpl-15) permits selected
 * Type/Template operands but requires complete producer inputs before selection.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) retains value
 * definition ancestry independently of whole-producer validation obligations.
 */
#include "internal.h"

/** Read the actual public symbol under the using definition path. The separate
 * required graph still validates every other result and statement. A suspended
 * creation has no fabricated definition or context to read.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves
 * definition cycles and cached incoming provenance.
 */
bool static_result_selection_read(StaticResultEvaluation *evaluation, StaticResultUse *use) {
    if (!use->source->selection || !use->producer) { return true; }
    if (!static_result_definition_use(evaluation->plan, use->producer, use->source->selection,
        use->definition_path, use->source->expression->selected_export, &use->selection)) { return false; }
    return static_result_push_definition(evaluation, use->selection);
}

/** A scalar receiver may already have an expected Type; therefore never equate
 * it to a symbolic supplier result. Both the full input key and the selected
 * value must be complete, and the definition must have a checked ready RHS.
 * [TPL-15](../../../../../spec/language/templates.md#tpl-15) forbids inverting
 * a producer, including one with an unused unresolved input.
 */
bool static_result_selection_equation(StaticResultPlan *plan, StaticResultUse *use) {
    if (!use->source->selection || !use->selection) { return true; }
    StaticRegistryUse *registered;
    StaticRegistryKeyState keys;
    if (!static_registry_use(plan->registry, use->producer->creation, &registered, &keys)) {
        (void)static_result_dependencies(plan); return false;
    }
    if (keys != STATIC_REGISTRY_KEY_CLOSED) { return true; }
    const StaticResultDefinitionUse *definition = use->selection;
    const StaticResultUse *rhs = definition->cache_hit ? definition->checked_rhs : definition->rhs;
    if (definition->outcome != STATIC_RESULT_READY || !rhs || rhs->outcome != STATIC_RESULT_READY || !rhs->closed) {
        return true;
    }
    const StaticTerm *value;
    StaticConstraintOutcome outcome;
    StaticConstraintSolver *solver = plan->guards->applications->solver;
    if (!static_constraint_value(solver, definition->definition->value, &value, &outcome)) {
        (void)static_result_dependencies(plan); return false;
    }
    if (outcome != STATIC_CONSTRAINT_COMPLETE) { return true; }
    if (!static_constraint_equate(solver, use->source->value, value, use->source->expression->selected_export)) {
        (void)static_result_dependencies(plan); return false;
    }
    return true;
}
