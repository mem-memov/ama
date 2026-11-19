/** @file
 * @brief Select shared fields only after independently closing their namespace and checked source scope.
 * [CONST-03](../../../../../spec/language/declarations.md#const-03) forbids receiver-driven namespace completion.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) closes projected namespace producers.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial selection metadata.
 */
#include <stdint.h>
#include "../internal.h"
#include "../../bindings/normalize.h"

/** Pointer identity precedes following a borrowed deferred read handle.
 * [CONST-03](../../../../../spec/language/declarations.md#const-03) preserves exact source reads.
 */
static bool registered(const StaticConstantPlan *plan, const StaticConstantRead *candidate) {
    for (const StaticConstantRead *read = plan->first_read; read; read = read->next) {
        if (read == candidate) { return true; }
    }
    return false;
}

/** Normalize actual own member names/positions only after independent namespace/supplier
 * closure, then bind symbolic declared Types without executing their values.
 * Existing selection reuse still rechecks current equations and namespace scope.
 * [CONST-03](../../../../../spec/language/declarations.md#const-03) separates namespace and member typing.
 */
bool static_constant_select(StaticConstantPlan *plan, const StaticConstantRead *read,
    const StaticResultContext *context, const StaticConstantSelection **output) {
    if (!static_constant_dependencies(plan)) { return false; }
    if (output == NULL || !registered(plan, read)) {
        return static_constant_fail(plan, STATIC_CONSTANT_INVALID, (SourceSpan){0}, "Invalid constant read selection handle or receiver");
    }
    const StaticTerm *namespace_type;
    if (!static_ordinary_resolve(plan->publications, read->source, &namespace_type)) {
        (void)static_constant_dependencies(plan); return false;
    }
    if (context == NULL && read->source->source) { context = read->source->source->producer; }
    if (!static_constant_scope(plan, context, namespace_type)) { return false; }
    for (const StaticConstantSelection *selection = plan->first_selection; selection; selection = selection->next) {
        if (selection->read == read && selection->context == context) { *output = selection; return true; }
    }
    if (plan->selection_count == SIZE_MAX) {
        return static_constant_fail(plan, STATIC_CONSTANT_OUT_OF_MEMORY, read->syntax->span,
            "Constant namespace selection count exceeds representable storage");
    }
    BindingInterface *view;
    BindingApplication *requests;
    if (!static_constant_surface(plan, namespace_type, &view) ||
        !binding_normalize_runtime(plan->publications->results->guards->applications->bindings, view, read->syntax, &requests)) {
        (void)static_constant_dependencies(plan); return false;
    }
    StaticConstantSelection *selection = static_constant_allocate(plan, sizeof(*selection));
    if (selection == NULL) { return false; }
    *selection = (StaticConstantSelection){.read = read, .context = context, .namespace_type = namespace_type, .requests = requests};
    if (!static_constant_members(plan, selection)) { return false; }
    selection->next = plan->first_selection;
    plan->first_selection = selection;
    plan->selection_count++;
    *output = selection;
    return true;
}
