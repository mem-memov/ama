/** @file
 * @brief Define operation-local iterative static evaluation frames and narrow phase interfaces.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) orders guarded dependencies.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) bounds scratch lifetime.
 */
#ifndef AMA_STATIC_RESULT_EVALUATE_INTERNAL_H
#define AMA_STATIC_RESULT_EVALUATE_INTERNAL_H
#include "../internal.h"

/** A continuation retains one source phase and its next original dependency.
 * Nested frames never consume the host call stack.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves dependency scope.
 */
typedef struct StaticResultFrame {
    struct StaticResultFrame *parent;
    StaticResultUse *expression;
    StaticResultDefinitionUse *definition;
    const BindingRequest *input;
    unsigned phase;
    bool pending;
    bool closed;
} StaticResultFrame;

/** Scratch frames expire at the end of one finite evaluation attempt.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) separates temporary state.
 */
typedef struct {
    StaticResultPlan *plan;
    SourceArena arena;
    StaticResultFrame *top;
} StaticResultEvaluation;

/** Push only inactive expression work; active equations defer without recursion.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves logical Pending.
 */
bool static_result_push_expression(StaticResultEvaluation *, StaticResultUse *);
/** Entered definition handles are cache checked before expanding their RHS.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) forbids cache bypass.
 */
bool static_result_push_definition(StaticResultEvaluation *, StaticResultDefinitionUse *);
/** Pop one completed frame, propagating readiness/closure to its continuation.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) separates structural and closed results.
 */
void static_result_pop(StaticResultEvaluation *, StaticResultOutcome, bool);
/** Advance one expression phase; descendants are pushed, never called recursively.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves lexical application.
 */
bool static_result_expression_step(StaticResultEvaluation *);
/** Advance one result/alias definition phase with distinct producer/value terms.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves selected results.
 */
bool static_result_definition_step(StaticResultEvaluation *);
/** Generate only actual output receiver equalities; receiver reads add no
 * definition dependency by themselves.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) separates equality and definition graphs.
 */
bool static_result_receivers(StaticResultPlan *, StaticResultUse *);
/** Query selected closure without demanding unrelated registered roots.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) requires complete producers.
 */
bool static_result_closed(StaticResultPlan *, const StaticTerm *, bool *);
/** Read the alias's exact defining foreign output under its own definition
 * ancestry. A suspended producer has no context and returns Pending without a
 * fabricated target. Root validation of other outputs remains independent.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves actual value edges.
 */
bool static_result_alias_read(StaticResultPlan *, StaticResultDefinitionUse *,
    StaticResultDefinitionUse **, bool *);
/** Query full named producer input and selected value closure without inventing
 * an expression value for the producer itself.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) closes every producer input.
 */
bool static_result_producer_closed(StaticResultPlan *, StaticResultUse *, bool *);
/** Read an exact declaration selector under its original definition ancestry.
 * [TPL-15](../../../../../spec/language/templates.md#tpl-15) keeps the selected
 * scalar separate from expected consumer facts until supplier input closure.
 */
bool static_result_selection_read(StaticResultEvaluation *, StaticResultUse *);
/** Drive an already initialized exact-root stack and always release its scratch
 * arena, including after push/step failure. Graph passes use their prepared root
 * handles rather than reconstructing a potentially different root occurrence.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns attempt cleanup.
 */
bool static_result_run(StaticResultEvaluation *, bool);
#endif
