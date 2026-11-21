/** @file
 * @brief Constrain a declaration selection by its actual finite structural definition.
 * [TPL-15](../../../../../spec/language/templates.md#tpl-15) permits selected
 * Type/Template operands and requires complete producer inputs before admission.
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

/** Equate the receiver with its independently identified structural definition.
 * These are simultaneous constraints, not publication: unused supplier inputs,
 * graph obligations and actual use provenance must still close separately.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) permits structural
 * evidence from selected outputs; [TPL-15](../../../../../spec/language/templates.md#tpl-15)
 * combines it with ordinary typed connections. Requiring a closed supplier here
 * would discard the very equations that can determine its inputs.
 */
bool static_result_selection_equation(StaticResultPlan *plan, const StaticResultSource *source,
    const StaticResultDefinitionUse *definition) {
    if (!source->selection || !definition) { return true; }
    const StaticResultUse *rhs = definition->cache_hit ? definition->checked_rhs : definition->rhs;
    if (!rhs) { return true; }
    if (!static_constraint_equate(plan->guards->applications->solver,
        source->value, definition->definition->value, source->expression->selected_export)) {
        (void)static_result_dependencies(plan); return false;
    }
    return true;
}
