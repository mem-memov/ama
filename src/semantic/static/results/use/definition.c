/** @file
 * @brief Preserve guarded definition occurrences before consulting any semantic value cache.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) checks dependency ancestry.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial records.
 */
#include <stdint.h>
#include "../internal.h"

/** Guard entry precedes even an existing occurrence lookup; actual source
 * result/alias symbols remain scoped by the exact target creation context.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) forbids cache bypass.
 */
bool static_result_definition_use(StaticResultPlan *plan, const StaticResultContext *context,
    const StaticSymbol *symbol, const StaticDefinitionPath *parent, SourceSpan site,
    StaticResultDefinitionUse **output) {
    if (!static_result_dependencies(plan)) { return false; }
    if (output == NULL || !static_result_has_context(plan, context)) {
        return static_result_fail(plan, STATIC_RESULT_INVALID, (SourceSpan){0}, "Invalid scoped definition-use context");
    }
    const StaticResultDefinition *definition;
    if (!static_result_definition(plan, context->creation->owner->environment, symbol, &definition)) { return false; }
    const StaticDefinitionPath *path;
    if (!static_definition_enter(plan->guards, parent, context->creation, symbol, site, &path)) {
        (void)static_result_dependencies(plan); return false;
    }
    for (StaticResultDefinitionUse *use = plan->first_definition_use; use; use = use->next) {
        if (use->definition == definition && use->context == context && use->path == path) { *output = use; return true; }
    }
    if (plan->definition_use_count == SIZE_MAX) {
        return static_result_fail(plan, STATIC_RESULT_OUT_OF_MEMORY, symbol->site,
            "Static definition-use count exceeds representable storage");
    }
    StaticResultDefinitionUse *use = static_result_allocate(plan, sizeof(*use));
    if (use == NULL) { return false; }
    *use = (StaticResultDefinitionUse){.definition = definition, .context = context, .path = path,
        .outcome = STATIC_RESULT_PENDING, .next = plan->first_definition_use};
    plan->first_definition_use = use;
    plan->definition_use_count++;
    *output = use;
    return true;
}
