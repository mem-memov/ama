/** @file
 * @brief Own iterative evaluation continuations and propagate actual child outcomes.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves guarded dependency work.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) releases temporary frames.
 */
#include "internal.h"

/** Checked scratch allocation never publishes a frame on failure.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial work.
 */
static StaticResultFrame *frame(StaticResultEvaluation *evaluation) {
    StaticResultFrame *value = source_arena_allocate(&evaluation->arena, sizeof(*value));
    if (value == NULL) {
        (void)static_result_fail(evaluation->plan, STATIC_RESULT_OUT_OF_MEMORY, (SourceSpan){0},
            "Static evaluation could not allocate a continuation");
        return NULL;
    }
    *value = (StaticResultFrame){.parent = evaluation->top, .closed = true};
    evaluation->top = value;
    return value;
}

/** An active source equation is unresolved work rather than an invented
 * definition cycle. Actual named definition cycles are rejected by T178 first.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) separates graphs.
 */
bool static_result_push_expression(StaticResultEvaluation *evaluation, StaticResultUse *use) {
    if (use->active) {
        if (evaluation->top) { evaluation->top->pending = true; evaluation->top->closed = false; }
        return true;
    }
    StaticResultFrame *value = frame(evaluation);
    if (value == NULL) { return false; }
    value->expression = use;
    use->active = true;
    return true;
}

/** Every caller enters the definition guard before requesting this stack push.
 * Cached values still propagate independently checked readiness to the parent.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves checked reuse.
 */
bool static_result_push_definition(StaticResultEvaluation *evaluation, StaticResultDefinitionUse *use) {
    bool hit;
    if (!static_result_cache_read(evaluation->plan, use, &hit)) { return false; }
    if (hit) { return true; }
    StaticResultFrame *value = frame(evaluation);
    if (value == NULL) { return false; }
    value->definition = use;
    use->active = true;
    return true;
}

/** Completion accumulates conjunctions; one unresolved child cannot be hidden
 * by a later ready sibling. Every active flag is cleared before cache storage.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) requires checked dependencies.
 */
void static_result_pop(StaticResultEvaluation *evaluation, StaticResultOutcome outcome, bool closed) {
    StaticResultFrame *value = evaluation->top;
    if (value->expression) {
        value->expression->active = false;
        value->expression->outcome = outcome;
        value->expression->closed = closed;
    } else {
        value->definition->active = false;
        value->definition->outcome = outcome;
    }
    evaluation->top = value->parent;
    if (evaluation->top) {
        evaluation->top->pending |= outcome == STATIC_RESULT_PENDING;
        evaluation->top->closed &= closed;
    }
}
