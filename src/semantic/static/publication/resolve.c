/** @file
 * @brief Publish an exact source occurrence only after complete producer and dependency closure.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) forbids partial producer publication.
 * [TPL-15](../../../../spec/language/templates.md#tpl-15) publishes a selected
 * declaration operand as its scalar value, retaining supplier proof in its use.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns partial construction.
 */
#include <stdint.h>
#include "internal.h"

/** Repeated publication rechecks current equations and guards before returning
 * an immutable descriptor. Later contradictory evidence cannot reuse stale data.
 * No runtime state or separate nominal output type is created.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) preserves exact identity.
 */
bool static_publication_resolve(StaticPublicationPlan *plan, const StaticResultContext *context,
    const AstTypeExpression *expression, const StaticPublication **output) {
    if (!static_publication_dependencies(plan)) { return false; }
    if (output == NULL) {
        return static_publication_fail(plan, STATIC_PUBLICATION_INVALID, (SourceSpan){0}, "Missing static publication receiver");
    }
    StaticResultUse *use;
    if (!static_result_use(plan->results, context, expression, NULL, &use)) {
        (void)static_publication_dependencies(plan); return false;
    }
    const StaticTerm *value;
    if (!static_publication_close(plan, use, &value)) { return false; }
    for (const StaticPublication *existing = plan->first; existing; existing = existing->next) {
        if (existing->use != use) { continue; }
        if (existing->value != value) {
            return static_publication_fail(plan, STATIC_PUBLICATION_CONFLICT, expression->span,
                "A static source publication changed its canonical value");
        }
        *output = existing;
        return true;
    }
    if (plan->count == SIZE_MAX) {
        return static_publication_fail(plan, STATIC_PUBLICATION_OUT_OF_MEMORY, expression->span,
            "Static publication count exceeds representable storage");
    }
    StaticPublication *publication = static_publication_allocate(plan, sizeof(*publication));
    if (publication == NULL) { return false; }
    *publication = (StaticPublication){.use = use,
        .context = use->source->producer && !use->source->selection ? use->producer : NULL, .value = value};
    if (!static_publication_outputs(plan, publication)) { return false; }
    publication->next = plan->first;
    plan->first = publication;
    plan->count++;
    *output = publication;
    return true;
}
