/** @file
 * @brief Retain exact expression and guarded definition dependencies for later producer closure.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves required source checks through caches.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial dependency records.
 */
#include <stdint.h>
#include "../internal.h"

/** Dependency identity is the actual visited handle, not its canonical value.
 * These edges do not reinterpret receiver equalities as definition dependencies.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) separates graph meanings.
 */
bool static_result_dependency(StaticResultPlan *plan, StaticResultUse *parent,
    const StaticResultUse *expression, const StaticResultDefinitionUse *definition) {
    if (!static_result_dependencies(plan)) { return false; }
    if (!static_result_has_use(plan, parent) || (expression == NULL) == (definition == NULL) ||
        (expression && !static_result_has_use(plan, expression)) ||
        (definition && !static_result_has_definition_use(plan, definition))) {
        return static_result_fail(plan, STATIC_RESULT_INVALID, (SourceSpan){0}, "Invalid static source dependency handles");
    }
    for (const StaticResultDependency *edge = parent->first_dependency; edge; edge = edge->next) {
        if (edge->expression == expression && edge->definition == definition) { return true; }
    }
    if (plan->dependency_count == SIZE_MAX) {
        return static_result_fail(plan, STATIC_RESULT_OUT_OF_MEMORY, parent->source->expression->span,
            "Static source dependency count exceeds representable storage");
    }
    StaticResultDependency *edge = static_result_allocate(plan, sizeof(*edge));
    if (edge == NULL) { return false; }
    *edge = (StaticResultDependency){.expression = expression, .definition = definition, .next = parent->first_dependency};
    parent->first_dependency = edge;
    plan->dependency_count++;
    return true;
}
