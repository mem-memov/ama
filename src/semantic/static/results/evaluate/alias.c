/** @file
 * @brief Read only an alias's exact defining foreign result under that alias's value ancestry.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) separates selected definition reads from unrelated output validation.
 */
#include "internal.h"

/** The first receiver's original syntax fixes its foreign slot; later equality
 * receipts cannot replace it. A suspended creation has no scope to read yet.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves immutable alias definitions.
 */
bool static_result_alias_read(StaticResultPlan *plan, StaticResultDefinitionUse *use,
    StaticResultDefinitionUse **output, bool *pending) {
    StaticResultUse *rhs = use->rhs;
    if (!rhs->source->producer || !use->definition->receiver) {
        return static_result_fail(plan, STATIC_RESULT_INVALID, use->definition->symbol->site,
            "Static alias has no original named producer receipt");
    }
    if (!rhs->producer) { *pending = true; return true; }
    for (const BindingRequest *request = rhs->requests->first; request; request = request->next) {
        if (request->direction != BINDING_STATIC_OUTPUT ||
            request->syntax.static_binding != use->definition->receiver) { continue; }
        StaticResultDefinitionUse *selected;
        if (!static_result_definition_use(plan, rhs->producer, request->slot->static_symbol,
            use->path, request->span, &selected)) { return false; }
        *output = selected;
        *pending = false;
        return true;
    }
    return static_result_fail(plan, STATIC_RESULT_INVALID, use->definition->symbol->site,
        "Static alias defining receipt has no actual foreign result slot");
}
