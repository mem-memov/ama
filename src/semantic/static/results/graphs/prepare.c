/** @file
 * @brief Queue every public definition and producer statement as an independent static validation root.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) includes public aliases and equality-only statements.
 */
#include "internal.h"

/** Source collection already fixed all public binders. Preparing their roots
 * with null definition ancestry avoids falsely making unrelated outputs depend
 * on whichever selected value happened to trigger graph validation.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) separates the two graphs.
 */
bool static_result_graph_prepare(StaticResultPlan *plan, StaticResultGraph *graph) {
    const StaticModule *module = graph->context->creation->owner->environment->definition;
    for (const StaticSymbol *symbol = module->first; symbol; symbol = symbol->next) {
        if (symbol->role != STATIC_RESULT && symbol->role != STATIC_ALIAS) { continue; }
        StaticResultDefinitionUse *definition;
        if (!static_result_definition_use(plan, graph->context, symbol, NULL, symbol->site, &definition) ||
            !static_result_graph_work(plan, graph, NULL, definition)) { return false; }
    }
    for (const StaticOccurrence *site = module->first_occurrence; site; site = site->next) {
        if (site->receiver != NULL || site->statement->kind != AST_STATIC_PROJECTION ||
            site->statement->as.static_projection != site->expression) { continue; }
        StaticResultUse *expression;
        if (!static_result_use(plan, graph->context, site->expression, NULL, &expression) ||
            !static_result_graph_work(plan, graph, expression, NULL)) { return false; }
    }
    return true;
}
