/** @file
 * @brief Guard actual compiler-only producer scopes before exposing their output definitions.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) suspends repeated unknown keys before expansion.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) separates producer inputs and returned values.
 */
#include "internal.h"

/** First entry consumes the immutable full input map. A suspended repeat has
 * no new environment/context. Later independent evidence can resume that same
 * request; a registered context instead refreshes its actual canonical use.
 * No nominal Type, static body value or executable admission is synthesized.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) retains actual incoming ancestry.
 */
bool static_result_apply_producer(StaticResultPlan *plan, StaticResultUse *use,
    StaticApplicationOutcome *output) {
    if (use->producer == NULL) {
        const StaticCreationPath *path = NULL;
        StaticCreationOutcome entered;
        const StaticResultProducer *request = use->producer_request;
        if (!static_creation_producer_enter(plan->guards, use->context->creation,
            use->source->expression, request->inputs, request->input_count, &path, &entered)) {
            (void)static_result_dependencies(plan); return false;
        }
        if (entered == STATIC_CREATION_PENDING) { *output = STATIC_APPLY_PENDING; return true; }
        if (!static_result_context(plan, path, &use->producer)) { return false; }
    }
    if (!static_result_require_graph(plan, use->producer, &use->required_graph)) { return false; }
    StaticRegistryUse *registered;
    StaticRegistryKeyState keys;
    if (!static_registry_use(plan->registry, use->producer->creation, &registered, &keys)) {
        (void)static_result_dependencies(plan); return false;
    }
    *output = keys == STATIC_REGISTRY_KEY_CLOSED ? STATIC_APPLY_RESOLVED : STATIC_APPLY_PENDING;
    return true;
}
