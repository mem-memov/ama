/**
 * @file
 * @brief Cache complete operand preparation by registered original source identity.
 *
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) publishes
 * only fully prepared cache entries. [TPL-11](../../../../../spec/language/templates.md#tpl-11)
 * leaves all actual application and expansion validation with expression uses.
 */
#include <stdint.h>
#include "internal.h"

/** Check source identity before following any borrowed source payload.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) requires an original
 * source/environment identity; a copied record is not another registered key.
 */
static bool registered(const StaticResultPlan *plan, const StaticResultSource *source) {
    for (const StaticResultSource *item = plan->first_source; item; item = item->next) {
        if (item == source) { return true; }
    }
    return false;
}

/** Reuse one immutable preparation or publish a complete new cache entry.
 * Lookup does not solve equations or change an actual use's provenance/state.
 * Failure keeps output/count/cache links unchanged; partial child owners remain
 * safely destructible under [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03).
 */
bool static_result_operands(StaticResultPlan *plan, const StaticResultSource *source,
    const StaticTermApplication **output) {
    if (!static_result_dependencies(plan)) { return false; }
    if (output == NULL || !registered(plan, source)) {
        return static_result_fail(plan, STATIC_RESULT_INVALID, (SourceSpan){0},
            "Invalid original operand source or receiver");
    }
    for (const StaticResultOperands *item = plan->first_operands; item; item = item->next) {
        if (item->source == source) { *output = item->prepared; return true; }
    }
    if (plan->operand_count == SIZE_MAX) {
        return static_result_fail(plan, STATIC_RESULT_OUT_OF_MEMORY, source->expression->span,
            "Static operand preparation count exceeds representable storage");
    }
    const StaticTermApplication *prepared;
    if (!static_result_collect_operands(plan, source, &prepared)) { return false; }
    StaticResultOperands *item = static_result_allocate(plan, sizeof(*item));
    if (item == NULL) { return false; }
    *item = (StaticResultOperands){.source = source, .prepared = prepared, .next = plan->first_operands};
    plan->first_operands = item;
    plan->operand_count++;
    *output = prepared;
    return true;
}
