/** @file
 * @brief Drive a single finite static evaluation attempt without recursive host calls or runtime execution.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) retains unresolved structural work.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) bounds continuation lifetime.
 */
#include "internal.h"

/** Both expression and independent definition roots share the same iterative
 * phases and unconditional scratch teardown.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) bounds each attempt.
 */
bool static_result_run(StaticResultEvaluation *evaluation, bool success) {
    while (success && evaluation->top) {
        success = evaluation->top->expression ? static_result_expression_step(evaluation) :
            static_result_definition_step(evaluation);
    }
    source_arena_destroy(&evaluation->arena);
    return success;
}

/** Pending is returned to the closure scheduler rather than retried without
 * evidence. Source/definition records persist; temporary continuations expire.
 * Failure leaves owned partial semantic work destructible and output untouched.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) forbids guessed completion.
 */
bool static_result_evaluate(StaticResultPlan *plan, const StaticResultContext *context,
    const AstTypeExpression *expression, StaticResultOutcome *output) {
    if (!static_result_dependencies(plan)) { return false; }
    if (output == NULL) {
        return static_result_fail(plan, STATIC_RESULT_INVALID, (SourceSpan){0}, "Missing static evaluation outcome receiver");
    }
    StaticResultGraph *graph;
    if (!static_result_require_graph(plan, context, &graph)) { return false; }
    StaticResultUse *use;
    if (!static_result_use(plan, context, expression, NULL, &use)) { return false; }
    StaticResultEvaluation evaluation = {.plan = plan};
    source_arena_init(&evaluation.arena, plan->arena.allocator);
    bool success = static_result_run(&evaluation, static_result_push_expression(&evaluation, use));
    if (success) { *output = use->outcome; }
    return success;
}

/** Independent receiver evidence uses a real definition root with its original
 * symbol introduction. Guard entry still precedes cached value reuse.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves acyclic definitions.
 */
bool static_result_evaluate_definition(StaticResultPlan *plan, const StaticResultContext *context,
    const StaticSymbol *symbol, StaticResultOutcome *output) {
    if (!static_result_dependencies(plan)) { return false; }
    if (output == NULL) {
        return static_result_fail(plan, STATIC_RESULT_INVALID, (SourceSpan){0}, "Missing static definition outcome receiver");
    }
    StaticResultGraph *graph;
    if (!static_result_require_graph(plan, context, &graph)) { return false; }
    StaticResultDefinitionUse *use;
    if (!static_result_definition_use(plan, context, symbol, NULL, (SourceSpan){0}, &use)) { return false; }
    StaticResultEvaluation evaluation = {.plan = plan};
    source_arena_init(&evaluation.arena, plan->arena.allocator);
    bool success = static_result_run(&evaluation, static_result_push_definition(&evaluation, use));
    if (success) { *output = use->outcome; }
    return success;
}
