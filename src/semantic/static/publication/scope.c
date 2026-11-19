/** @file
 * @brief Publish a real static-module root without inventing a producer value or call.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) requires complete inputs and public definitions.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns partial descriptors.
 */
#include <stdint.h>
#include "internal.h"

/** Root bundles are keyed by their actual registered context. A repeated request
 * rechecks closure before returning immutable metadata, never an ordinary instance.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12).
 */
bool static_publication_scope(StaticPublicationPlan *plan, const StaticResultContext *context,
    const StaticPublication **output) {
    if (!static_publication_dependencies(plan)) { return false; }
    if (!output || !static_result_has_context(plan->results, context)) {
        return static_publication_fail(plan, STATIC_PUBLICATION_INVALID, (SourceSpan){0},
            "Invalid static publication root or receiver");
    }
    StaticResultGraph *graph;
    if (!static_result_require_graph(plan->results, context, &graph)) {
        (void)static_publication_dependencies(plan); return false;
    }
    if (!graph) {
        return static_publication_fail(plan, STATIC_PUBLICATION_INVALID, context->creation->site,
            "A static publication root must be a static module");
    }
    if (!static_publication_close_graph(plan, graph)) { return false; }
    for (const StaticPublication *existing = plan->first; existing; existing = existing->next) {
        if (!existing->use && existing->context == context) { *output = existing; return true; }
    }
    if (plan->count == SIZE_MAX) {
        return static_publication_fail(plan, STATIC_PUBLICATION_OUT_OF_MEMORY, context->creation->site,
            "Static publication count exceeds representable storage");
    }
    StaticPublication *publication = static_publication_allocate(plan, sizeof(*publication));
    if (!publication) { return false; }
    publication->context = context;
    if (!static_publication_outputs(plan, publication)) { return false; }
    publication->next = plan->first;
    plan->first = publication;
    plan->count++;
    *output = publication;
    return true;
}
