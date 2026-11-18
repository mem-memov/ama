/** @file
 * @brief Register exact guarded required scopes without manufacturing module roots.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) preserves required provenance.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial records.
 */
#include <stdint.h>
#include "../internal.h"

/** Payload equality does not authorize a copied or foreign context.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) preserves ownership.
 */
bool static_result_has_context(const StaticResultPlan *plan, const StaticResultContext *context) {
    for (const StaticResultContext *item = plan->first_context; item; item = item->next) {
        if (item == context) { return true; }
    }
    return false;
}

/** Equal owner keys can have distinct actual incoming paths; retain them all.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) preserves ancestry.
 */
bool static_result_context(StaticResultPlan *plan, const StaticCreationPath *creation, const StaticResultContext **output) {
    if (!static_result_dependencies(plan)) { return false; }
    if (output == NULL || !static_guard_has_path(plan->guards, creation)) {
        return static_result_fail(plan, STATIC_RESULT_INVALID, (SourceSpan){0}, "Invalid required static evaluation scope");
    }
    for (const StaticResultContext *context = plan->first_context; context; context = context->next) {
        if (context->creation == creation) { *output = context; return true; }
    }
    if (plan->context_count == SIZE_MAX) {
        return static_result_fail(plan, STATIC_RESULT_OUT_OF_MEMORY, creation->site,
            "Static result scope count exceeds representable storage");
    }
    StaticResultContext *context = static_result_allocate(plan, sizeof(*context));
    if (context == NULL) { return false; }
    *context = (StaticResultContext){.creation = creation, .next = plan->first_context};
    plan->first_context = context;
    plan->context_count++;
    *output = context;
    return true;
}
