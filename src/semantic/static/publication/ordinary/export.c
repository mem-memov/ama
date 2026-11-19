/** @file
 * @brief Resolve a static export independently before admitting any consumer Type equality.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) requires complete suppliers and all public outputs.
 * [TYPE-01](../../../../../spec/language/declarations.md#type-01) excludes returned Templates from ordinary storage.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) retains required validation provenance.
 */
#include "internal.h"

/** A concrete path supplies a compilation root, not runtime storage or arguments
 * inferred from the using field. Retain a validation dependency without inventing
 * a constructor application or source-language creation edge.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) separates exported identity and executable admission.
 */
static bool root(StaticPublicationPlan *plan, StaticOrdinaryType *type) {
    if (!type->producer_context) {
        const StaticTermEnvironment *environment;
        const StaticCreationPath *path;
        if (!static_term_environment(plan->results->guards->applications->terms, type->producer, NULL, &environment) ||
            !static_creation_root(plan->results->guards, environment, &path) ||
            !static_result_context(plan->results, path, &type->producer_context)) {
            (void)static_publication_dependencies(plan); return false;
        }
    }
    if (type->context && !static_registry_depend(plan->results->registry, type->context->registry_use,
        type->producer_context->registry_use, type->expression->span)) {
        (void)static_publication_dependencies(plan); return false;
    }
    return true;
}

/** Closure precedes selection and equality. Consumer constraints therefore
 * cannot bind a missing supplier input or choose among its specializations.
 * Other returned Templates are permitted; only the selected value must be Type.
 * [TYPE-01](../../../../../spec/language/declarations.md#type-01) fixes this phase boundary.
 */
bool static_ordinary_export(StaticPublicationPlan *plan, StaticOrdinaryType *type) {
    if (!root(plan, type)) { static_ordinary_error_context(plan, type); return false; }
    const StaticPublication *publication;
    if (!static_publication_scope(plan, type->producer_context, &publication)) {
        static_ordinary_error_context(plan, type);
        return false;
    }
    const StaticPublishedOutput *selected = publication->first_output;
    while (selected && selected->symbol != type->selected) { selected = selected->next; }
    if (!selected) {
        return static_ordinary_export_failure(plan, type, STATIC_PUBLICATION_INVALID,
            "Closed static publication has no original selected export", NULL);
    }
    if (selected->value->kind != STATIC_TERM_NOMINAL || selected->value->category != STATIC_CATEGORY_TYPE) {
        return static_ordinary_export_failure(plan, type, STATIC_PUBLICATION_CONFLICT,
            "An ordinary export selector requires a complete Type rather than a Template", selected->value);
    }
    StaticConstraintSolver *solver = plan->results->guards->applications->solver;
    StaticConstraintOutcome solved;
    if (!static_constraint_equate(solver, type->value, selected->value, type->expression->selected_export) ||
        !static_constraint_solve(solver, &solved)) {
        (void)static_publication_dependencies(plan); return false;
    }
    type->publication = publication;
    return true;
}
