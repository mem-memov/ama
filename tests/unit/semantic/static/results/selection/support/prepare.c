/** @file
 * @brief Prepare original declaration selectors and advance their exact guarded supplier graphs.
 */
#include "../../common.c"

/** Load one actual declaration selector without invoking its later publication
 * consumer. This isolates foreign selection from ordinary storage admission.
 */
static const StaticResultContext *setup_selection(const char *caller, const char *target) {
    setup_guards(caller, target);
    initialize_results();
    const StaticCreationPath *path;
    const StaticResultContext *context;
    assert(static_creation_root(&guards, caller_environment, &path));
    assert(static_result_context(&results, path, &context));
    return context;
}

/** Exact source-backed field forms shared by scalar and partial result cases. */
static const char *selected_caller = "@!\ndata\n/test/fixture/v1.0.0/src/other\n .slot < //integer\n .out\nanchor&\n//integer\n";

/** Drive selected value reads and every required supplier graph. A bounded
 * number of fixture passes is an assertion budget, never an implementation
 * algorithm or acceptance of an unresolved result.
 */
static bool evaluate_selection(const StaticResultContext *context, const AstTypeExpression *expression,
    StaticResultUse **output) {
    if (!static_result_use(&results, context, expression, NULL, output)) { return false; }
    for (size_t pass = 0; pass < 4; pass++) {
        StaticResultOutcome outcome;
        if (!static_result_evaluate(&results, context, expression, &outcome)) { return false; }
        for (StaticResultGraph *graph = results.first_graph; graph; graph = graph->next) {
            if (!static_result_graph_pass(&results, graph, &outcome)) { return false; }
        }
    }
    return true;
}
