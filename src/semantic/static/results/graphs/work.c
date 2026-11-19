/** @file
 * @brief Retain one whole-graph validation root without inventing a value-read edge.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves independent validation obligations.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns unpublished graph work.
 */
#include <assert.h>
#include <stdint.h>
#include "internal.h"

/** Overflow is refused before linking; complete roots retain exact use handles.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) keeps partial graph preparation destructible.
 */
bool static_result_graph_work(StaticResultPlan *plan, StaticResultGraph *graph,
    StaticResultUse *expression, StaticResultDefinitionUse *definition) {
    assert((expression != NULL) != (definition != NULL));
    if (graph->work_count == SIZE_MAX) {
        return static_result_fail(plan, STATIC_RESULT_OUT_OF_MEMORY, graph->context->creation->site,
            "Static graph validation root count exceeds representable storage");
    }
    StaticResultGraphWork *work = static_result_allocate(plan, sizeof(*work));
    if (!work) { return false; }
    *work = (StaticResultGraphWork){.expression = expression, .definition = definition};
    if (graph->last) { graph->last->next = work; }
    else { graph->first = work; }
    graph->last = work;
    graph->work_count++;
    return true;
}
