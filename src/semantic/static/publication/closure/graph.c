/** @file
 * @brief Close the exact reachable source and receiver-proof graph without rejecting equality cycles.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) requires original producer closure through caches.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) bounds attempt worklists.
 */
#include "internal.h"

/** Actual source handles, not names, determine membership. A repeated receiver
 * relationship is visited once rather than recursively imposing false cycles.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) separates dependency meanings.
 */
bool static_publication_enqueue(StaticPublicationWork *work, const StaticResultUse *use) {
    if (!static_result_has_use(work->plan->results, use)) {
        return static_publication_fail(work->plan, STATIC_PUBLICATION_INVALID, (SourceSpan){0},
            "Required static closure proof has no registered source use");
    }
    for (const StaticPublicationWorkNode *node = work->first; node; node = node->next) {
        if (node->use == use) { return true; }
    }
    StaticPublicationWorkNode *node = source_arena_allocate(&work->arena, sizeof(*node));
    if (node == NULL) {
        return static_publication_fail(work->plan, STATIC_PUBLICATION_OUT_OF_MEMORY, use->source->expression->span,
            "Static producer closure could not allocate a source work item");
    }
    *node = (StaticPublicationWorkNode){.use = use};
    if (work->last) { work->last->next = node; }
    else { work->first = node; }
    work->last = node;
    return true;
}

/** A cached value retains its original checked RHS witness. Its selected value
 * alone cannot excuse an unresolved producer or transitive receiver requirement.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves cached provenance.
 */
bool static_publication_definition(StaticPublicationWork *work, const StaticResultDefinitionUse *definition) {
    if (!static_result_has_definition_use(work->plan->results, definition)) {
        return static_publication_fail(work->plan, STATIC_PUBLICATION_INVALID, (SourceSpan){0},
            "Required static closure proof has no guarded definition use");
    }
    StaticConstraintOutcome closed;
    const StaticTerm *value;
    if (!static_constraint_value(work->plan->results->guards->applications->solver,
        definition->definition->value, &value, &closed)) {
        (void)static_publication_dependencies(work->plan); return false;
    }
    work->closed &= closed == STATIC_CONSTRAINT_COMPLETE && definition->outcome == STATIC_RESULT_READY;
    return static_publication_enqueue(work, definition->cache_hit ? definition->checked_rhs : definition->rhs);
}

/** Visit original input, guarded head/output and independent receiver roots.
 * No unrelated registered root enters this queue. Explicit visited identity
 * handles permitted receiver equality cycles without a recursive closure rule.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) closes all required producers.
 */
bool static_publication_dependencies_closed(StaticPublicationPlan *plan, StaticResultUse *use, bool *output) {
    StaticPublicationWork work = {.plan = plan, .closed = true};
    source_arena_init(&work.arena, plan->arena.allocator);
    bool success = static_publication_enqueue(&work, use);
    for (const StaticPublicationWorkNode *node = work.first; success && node; node = node->next) {
        work.closed &= node->use->outcome == STATIC_RESULT_READY && node->use->closed;
        success = static_publication_evidence(&work, node->use);
        for (const StaticResultDependency *edge = node->use->first_dependency; success && edge; edge = edge->next) {
            success = edge->expression ? static_publication_enqueue(&work, edge->expression) :
                static_publication_definition(&work, edge->definition);
        }
    }
    source_arena_destroy(&work.arena);
    if (success) { *output = work.closed; }
    return success;
}
