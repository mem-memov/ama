/** @file
 * @brief Prepare Type-category field terms without evaluating original Type expressions or values.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) permits unresolved required fields.
 * [TPL-04](../../../../../../spec/language/templates.md#tpl-04) preserves per-owner field equations.
 * [ARCH-03](../../../../../../spec/implementation/compiler.md#arch-03) owns partial association records.
 */
#include <stdint.h>
#include "../../internal.h"

/** Original explicit expressions supply source result variables. Runtime receiver/
 * foreach identities supply separate Type holes; every association requires a
 * module Type even when storage starts empty or its default is never read.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) forbids value/literal Type anchors.
 */
static bool field(StaticConnectionPlan *plan, StaticTypedScope *scope, const StaticRuntimeField *definition) {
    if (scope->field_count == SIZE_MAX) {
        return static_connection_fail(plan, STATIC_CONNECTION_OUT_OF_MEMORY, definition->site, "Typed field association count exceeds representable storage");
    }
    StaticResultPlan *results = plan->constants->publications->results;
    StaticConstraintSolver *solver = results->guards->applications->solver;
    StaticOrdinaryType *type_source = NULL;
    const StaticTerm *type;
    if (definition->type_expression) {
        if (!static_ordinary_source(plan->constants->publications, scope->environment, definition->type_expression, &type_source)) {
            (void)static_connection_dependencies(plan); return false;
        }
        type = type_source->value;
    } else if (!static_term_runtime_type(solver->terms, scope->environment, definition->statement, definition->receiver, &type)) {
        (void)static_connection_dependencies(plan); return false;
    }
    if (!static_constraint_describe(solver, type, STATIC_CATEGORY_TYPE, NULL, definition->site) ||
        !static_constraint_require(solver, type, definition->site)) {
        (void)static_connection_dependencies(plan); return false;
    }
    StaticTypedField *value = static_connection_allocate(plan, sizeof(*value));
    if (value == NULL) { return false; }
    *value = (StaticTypedField){.definition = definition, .type = type, .type_source = type_source};
    if (scope->last_field) { scope->last_field->next = value; }
    else { scope->first_field = value; }
    scope->last_field = value;
    scope->field_count++;
    return true;
}

/** Consume each original lexical binding once, retaining identical source order
 * for exact corresponding-field joins across environments. No definition unfolds.
 * [TPL-04](../../../../../../spec/language/templates.md#tpl-04) fixes original field identity across uses.
 */
bool static_typed_fields(StaticConnectionPlan *plan, StaticTypedScope *scope) {
    for (const StaticRuntimeField *definition = scope->runtime->first_field; definition; definition = definition->next) {
        if (!field(plan, scope, definition)) { return false; }
    }
    return true;
}
