/** @file
 * @brief Retain every actual incoming context separately from source-only typed metadata.
 * [TPL-11](../../../../../../spec/language/templates.md#tpl-11) preserves creation/dependency provenance.
 * [ARCH-03](../../../../../../spec/implementation/compiler.md#arch-03) owns partial context attachment.
 */
#include <stdint.h>
#include "../../internal.h"

/** Actual membership is validated before payload access; metadata environments
 * never invent actual contexts. Each actual path has one retained attachment.
 * [TPL-11](../../../../../../spec/language/templates.md#tpl-11) forbids cache-based validation bypass.
 */
bool static_connection_context(StaticConnectionPlan *plan, const StaticResultContext *actual, const StaticTypedScope **output) {
    if (!static_connection_dependencies(plan)) { return false; }
    if (output == NULL || !static_result_has_context(plan->constants->publications->results, actual)) {
        return static_connection_fail(plan, STATIC_CONNECTION_INVALID, (SourceSpan){0}, "Invalid actual typed context or receiver");
    }
    for (const StaticTypedContext *context = plan->first_context; context; context = context->next) {
        if (context->actual == actual) { *output = context->scope; return true; }
    }
    if (plan->context_count == SIZE_MAX) {
        return static_connection_fail(plan, STATIC_CONNECTION_OUT_OF_MEMORY, actual->creation->site, "Typed actual-context count exceeds representable storage");
    }
    const StaticTypedScope *scope;
    if (!static_connection_scope(plan, actual->creation->owner->environment, &scope)) { return false; }
    StaticTypedContext *context = static_connection_allocate(plan, sizeof(*context));
    if (context == NULL) { return false; }
    *context = (StaticTypedContext){.actual = actual, .scope = scope, .next = plan->first_context};
    plan->first_context = context;
    plan->context_count++;
    *output = scope;
    return true;
}
