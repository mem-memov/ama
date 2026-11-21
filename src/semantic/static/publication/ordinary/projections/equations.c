/** @file
 * @brief Refine declaration selections without requiring unresolved suppliers to close prematurely.
 * [TPL-15](../../../../../../spec/language/templates.md#tpl-15) permits only
 * complete selected scalar equations and requires ordinary final storage Types.
 * [TPL-12](../../../../../../spec/language/static-results.md#tpl-12) retains
 * all supplier public definitions independently of the selected value read.
 * [COMP-04](../../../../../../spec/implementation/compiler.md#comp-04) preserves
 * actual selector and residual-formal diagnostic provenance.
 */
#include "internal.h"

/** Root declarations require an ordinary Type; nested operands may retain a
 * complete partial constructor. Check only independently closed supplier keys
 * and selected values, before scalar equality can obscure the actual residual.
 * [TPL-15](../../../../../../spec/language/templates.md#tpl-15) distinguishes
 * intermediate Template metadata from the final runtime-capable Type.
 */
static bool root_value(StaticPublicationPlan *plan, StaticOrdinaryType *type, StaticOrdinaryProjection *projection) {
    if (projection->source != type->annotation) { return true; }
    StaticRegistryUse *registered;
    StaticRegistryKeyState keys;
    if (!static_registry_use(plan->results->registry, projection->context->creation, &registered, &keys)) { return false; }
    if (keys != STATIC_REGISTRY_KEY_CLOSED) { return true; }
    const StaticTerm *value;
    StaticConstraintOutcome known;
    if (!static_constraint_value(plan->results->guards->applications->solver,
        projection->selected->definition->value, &value, &known)) { return false; }
    if (known != STATIC_CONSTRAINT_COMPLETE) { return true; }
    if (value->kind != STATIC_TERM_NOMINAL || value->category != STATIC_CATEGORY_TYPE) {
        return static_ordinary_export_failure(plan, type, STATIC_PUBLICATION_CONFLICT,
            "An ordinary export selector requires a complete Type rather than a Template", value);
    }
    if (value->as.nominal.origin->definition->syntax.sections.kind == MODULE_MARKER) {
        return static_ordinary_export_failure(plan, type, STATIC_PUBLICATION_CONFLICT,
            "A static marker export cannot supply runtime storage", value);
    }
    return true;
}

/** Drain the finite required graph queue, including newly appended witnesses.
 * Each graph pass uses the existing explicit evaluator stack. Unknown recursive
 * entries suspend through the creation guard before another scope is created.
 * [TPL-11](../../../../../../spec/language/templates.md#tpl-11) bounds expansion;
 * [TPL-12](../../../../../../spec/language/static-results.md#tpl-12) preserves
 * complete graph validation separately from selected definition ancestry.
 */
static bool graphs(StaticPublicationPlan *plan) {
    for (StaticResultGraph *graph = plan->results->first_graph; graph; graph = graph->next) {
        StaticResultOutcome outcome;
        if (!static_result_graph_pass(plan->results, graph, &outcome)) { return false; }
    }
    return true;
}

/** Read the original foreign symbol in its guarded supplier context. Pending
 * is ordinary inference work, not a hard unresolved error in this fact pass.
 * The eventual actual use must retain and close its own incoming proof.
 * [TPL-15](../../../../../../spec/language/templates.md#tpl-15) forbids inverse
 * inference; [COMP-04](../../../../../../spec/implementation/compiler.md#comp-04)
 * retains requiring-site context for an original root selector.
 */
bool static_ordinary_projection_equations(StaticPublicationPlan *plan, StaticOrdinaryType *type,
    const StaticResultSource *source, StaticOrdinaryProjection **output) {
    if (!static_ordinary_projection_prepare(plan, type, source, output)) { goto failure; }
    StaticOrdinaryProjection *projection = *output;
    if (!projection->context) { return true; }
    StaticResultOutcome outcome;
    if (!graphs(plan) ||
        !static_result_evaluate_definition(plan->results, projection->context, source->selection, &outcome) ||
        !static_result_definition_use(plan->results, projection->context, source->selection, NULL,
            source->expression->selected_export, &projection->selected)) { goto failure; }
    if (!root_value(plan, type, projection)) {
        if (plan->status == STATIC_PUBLICATION_OK) { goto failure; }
        return false;
    }
    if (!static_result_selection_equation(plan->results, source, projection->context, projection->selected)) { goto failure; }
    return true;
failure:
    (void)static_publication_dependencies(plan);
    if (type->producer) { static_ordinary_error_context(plan, type); }
    return false;
}
