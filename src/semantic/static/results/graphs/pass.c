/** @file
 * @brief Advance independent graph roots without turning validation cycles into recursive value waits.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) checks every public definition and equation statement.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) separates canonical checking and actual path proof.
 */
#include "internal.h"
#include "../evaluate/internal.h"

/** Cache hits retain the checked original RHS; a grounded receiver alone does
 * not certify that a definition's producing inputs or reads passed checking.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves complete producers.
 */
static bool definition_closed(StaticResultPlan *plan, const StaticResultDefinitionUse *definition,
    bool *output) {
    const StaticResultUse *rhs = definition->cache_hit ? definition->checked_rhs : definition->rhs;
    bool value_closed;
    if (!static_result_closed(plan, definition->definition->value, &value_closed)) { return false; }
    *output = definition->outcome == STATIC_RESULT_READY && value_closed && rhs &&
        rhs->outcome == STATIC_RESULT_READY && rhs->closed;
    return true;
}

/** Work identity is already finite and prepared. Each root uses the explicit
 * value stack; newly required child graphs are queued separately, not called
 * recursively here. All source-local checks may close while ordinary body checks
 * remain pending; neither the local flag nor a cache hit admits executable code.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12).
 */
bool static_result_graph_pass(StaticResultPlan *plan, StaticResultGraph *graph,
    StaticResultOutcome *output) {
    if (!static_result_dependencies(plan)) { return false; }
    bool owned = false;
    for (const StaticResultGraph *item = plan->first_graph; item; item = item->next) {
        if (item == graph) { owned = true; break; }
    }
    if (!output || !owned) {
        return static_result_fail(plan, STATIC_RESULT_INVALID, (SourceSpan){0}, "Invalid static graph pass or outcome receiver");
    }
    StaticRegistryKeyState state;
    if (!static_registry_scope_environment(plan->registry, graph->context->registry_use, &state)) {
        (void)static_result_dependencies(plan); return false;
    }
    if (graph->local_complete) { *output = STATIC_RESULT_READY; return true; }
    bool closed = true;
    for (StaticResultGraphWork *work = graph->first; work; work = work->next) {
        StaticResultEvaluation evaluation = {.plan = plan};
        source_arena_init(&evaluation.arena, plan->arena.allocator);
        bool pushed = work->definition ? static_result_push_definition(&evaluation, work->definition) :
            static_result_push_expression(&evaluation, work->expression);
        if (!static_result_run(&evaluation, pushed)) { return false; }
        bool local_closed;
        if (work->definition) {
            if (!definition_closed(plan, work->definition, &local_closed)) { return false; }
        } else {
            local_closed = work->expression->outcome == STATIC_RESULT_READY && work->expression->closed;
        }
        closed &= local_closed;
    }
    if (!static_registry_scope_environment(plan->registry, graph->context->registry_use, &state)) {
        (void)static_result_dependencies(plan); return false;
    }
    closed &= state == STATIC_REGISTRY_KEY_CLOSED;
    if (closed) {
        if (!static_registry_complete_local(plan->registry, graph->context->registry_use)) {
            (void)static_result_dependencies(plan); return false;
        }
        graph->local_complete = true;
    }
    *output = closed ? STATIC_RESULT_READY : STATIC_RESULT_PENDING;
    return true;
}
