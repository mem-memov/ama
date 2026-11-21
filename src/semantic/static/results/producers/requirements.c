/** @file
 * @brief Keep every actual producer input mandatory independently of selected outputs.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) requires the
 * complete input map even when an output discards an input. Recording those
 * requirements before saturation preserves exact failure provenance under
 * [COMP-04](../../../../../spec/implementation/compiler.md#comp-04), without
 * preventing [TPL-10](../../../../../spec/language/templates.md#tpl-10) inference.
 */
#include "internal.h"
bool static_result_require_producer_inputs(StaticResultPlan *plan, const StaticResultProducer *request) {
    const StaticResultSource *source = request->source;
    SourceSpan site = source->selection ? source->expression->selected_export : source->expression->head;
    for (size_t index = 0; index < request->input_count; index++) {
        if (!static_constraint_require(plan->guards->applications->solver, request->inputs[index].value, site)) {
            (void)static_result_dependencies(plan); return false;
        }
    }
    return true;
}
