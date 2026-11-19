/** @file
 * @brief Refine actual source and root-graph metadata to a finite evidence-driven fixed point.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) requires complete producers without arbitrary pass budgets.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) permits only structural inference facts.
 */
#include <assert.h>
#include "progress.h"

/** Drain newly required witnesses too; each value walk uses an explicit stack.
 * Whole-graph validation is not recursively placed on a value-definition path.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12).
 */
static bool graphs(StaticPublicationPlan *plan) {
    for (StaticResultGraph *graph = plan->results->first_graph; graph; graph = graph->next) {
        StaticResultOutcome outcome;
        if (!static_result_graph_pass(plan->results, graph, &outcome)) {
            (void)static_publication_dependencies(plan); return false;
        }
    }
    return true;
}

/** The selected closure is checked independently of unrelated pending roots.
 * All actual graph work can supply inference evidence, while only the selected
 * source/producer requirements must close for this immutable metadata result.
 * Unknown producer identity is never replaced by a guessed ordinary Type.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12).
 */
static bool refine(StaticPublicationPlan *plan, StaticResultUse *use, StaticResultGraph *graph,
    const StaticTerm **output) {
    assert((use != NULL) != (graph != NULL));
    StaticConstraintSolver *solver = plan->results->guards->applications->solver;
    SourceSpan site = use ? use->source->expression->span : graph->context->creation->site;
    StaticPublicationItem root = use ?
        (StaticPublicationItem){.kind = STATIC_CLOSURE_SOURCE, .as.source = use} :
        (StaticPublicationItem){.kind = STATIC_CLOSURE_GRAPH, .as.graph = graph};
    for (;;) {
        StaticPublicationProgress before = static_publication_progress(plan);
        if (use) {
            StaticResultOutcome outcome;
            if (!static_result_evaluate(plan->results, use->context, use->source->expression, &outcome)) {
                (void)static_publication_dependencies(plan); return false;
            }
        }
        if (!graphs(plan)) { return false; }
        bool dependencies_closed;
        if (!static_publication_dependencies_closed(plan, root, &dependencies_closed)) { return false; }
        const StaticTerm *value = NULL;
        StaticConstraintOutcome closed = STATIC_CONSTRAINT_COMPLETE;
        if (use && use->source->value && !static_constraint_value(solver, use->source->value, &value, &closed)) {
            (void)static_publication_dependencies(plan); return false;
        }
        if (dependencies_closed && closed == STATIC_CONSTRAINT_COMPLETE) {
            if (output) { *output = value; }
            return true;
        }
        if (static_publication_same_progress(before, static_publication_progress(plan))) {
            return static_publication_fail(plan, STATIC_PUBLICATION_UNRESOLVED, site,
                "Static producer inputs or required public definitions remain unresolved");
        }
    }
}

/** A named producer returns no own value; null is its successful bundle value,
 * never a fallback Type or a hidden default instance.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12).
 */
bool static_publication_close(StaticPublicationPlan *plan, StaticResultUse *use, const StaticTerm **output) {
    return refine(plan, use, NULL, output);
}

/** A real compiler-requested producer root needs no fabricated call syntax.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12).
 */
bool static_publication_close_graph(StaticPublicationPlan *plan, StaticResultGraph *graph) {
    return refine(plan, NULL, graph, NULL);
}
