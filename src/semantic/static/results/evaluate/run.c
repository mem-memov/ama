/** @file
 * @brief Drive a single finite static evaluation attempt without recursive host calls or runtime execution.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) retains unresolved structural work.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) bounds continuation lifetime.
 */
#include "internal.h"

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
    StaticResultUse *use;
    if (!static_result_use(plan, context, expression, NULL, &use)) { return false; }
    StaticResultEvaluation evaluation = {.plan = plan};
    source_arena_init(&evaluation.arena, plan->arena.allocator);
    bool success = static_result_push_expression(&evaluation, use);
    while (success && evaluation.top) {
        success = evaluation.top->expression ? static_result_expression_step(&evaluation) :
            static_result_definition_step(&evaluation);
    }
    source_arena_destroy(&evaluation.arena);
    if (success) { *output = use->outcome; }
    return success;
}
