/** @file
 * @brief Prepare named declaration suppliers under exact lexical creation provenance.
 * [TPL-15](../../../../../../spec/language/templates.md#tpl-15) retains every
 * named input, including stable omitted-input variables.
 * [TPL-11](../../../../../../spec/language/templates.md#tpl-11) guards entry
 * before following supplier definitions without inventing ordinary roots.
 * [ARCH-03](../../../../../../spec/implementation/compiler.md#arch-03) owns
 * partial source/request caches after failure or successful Pending.
 */
#include "internal.h"

/** Use actual incoming provenance when available. Source-only annotations
 * retain a graph predecessor that cannot be promoted into an ordinary context.
 * All nested operands share this same lexical owner under
 * [TPL-11](../../../../../../spec/language/templates.md#tpl-11).
 */
static bool parent(StaticPublicationPlan *plan, StaticOrdinaryType *type, const StaticCreationPath **output) {
    if (type->context) { *output = type->context->creation; return true; }
    if (!type->annotation_path && !static_creation_annotation(plan->results->guards,
        type->environment, type->expression, &type->annotation_path)) { return false; }
    *output = type->annotation_path;
    return true;
}

/** Cache only original immutable request identity. A guarded Pending keeps
 * this cache but publishes no new target context; independent evidence can
 * resume the same source occurrence with the same input variables.
 * [TPL-15](../../../../../../spec/language/templates.md#tpl-15) preserves
 * full producer-input obligations independently of the selected scalar result.
 */
bool static_ordinary_projection_prepare(StaticPublicationPlan *plan, StaticOrdinaryType *type,
    const StaticResultSource *source, StaticOrdinaryProjection **output) {
    if (!*output) {
        const StaticResultProducer *request;
        if (!static_result_prepare_producer(plan->results, source, &request)) { return false; }
        StaticOrdinaryProjection *projection = static_publication_allocate(plan, sizeof(*projection));
        if (!projection) { return false; }
        projection->source = source;
        projection->request = request;
        *output = projection;
    }
    StaticOrdinaryProjection *projection = *output;
    if (!projection->context) {
        const StaticCreationPath *lexical, *path;
        if (!parent(plan, type, &lexical)) { return false; }
        StaticCreationOutcome entered;
        if (!static_creation_producer_enter(plan->results->guards, lexical, source->expression,
            projection->request->inputs, projection->request->input_count, &path, &entered)) { return false; }
        if (entered == STATIC_CREATION_PENDING) { return true; }
        if (!static_result_context(plan->results, path, &projection->context)) { return false; }
    }
    return static_result_require_graph(plan->results, projection->context, &projection->graph);
}
