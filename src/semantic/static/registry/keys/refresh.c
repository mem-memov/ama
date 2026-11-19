/** @file
 * @brief Rebind required pending uses after independent constraints settle and recheck actual guard graphs.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) retains late-key obligations.
 * [TPL-05](../../../../../spec/language/templates.md#tpl-05) registers identities before ordinary validation.
 */
#include "../internal.h"

/** All uses are retained; a closed association never replaces a scoped path.
 * This is only a key-closure query, not an admission or source-body proof.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11).
 */
bool static_registry_refresh_keys(StaticRegistryPlan *plan, StaticRegistryKeyState *output) {
    if (!static_registry_dependencies(plan)) { return false; }
    if (output == NULL) {
        return static_registry_fail(plan, STATIC_REGISTRY_INVALID, (SourceSpan){0}, "Missing registry key-closure receiver");
    }
    StaticGuardClosure checked;
    if (!static_guard_refresh(plan->guards, &checked)) { (void)static_registry_dependencies(plan); return false; }
    StaticRegistryKeyState state = STATIC_REGISTRY_KEY_CLOSED;
    for (StaticRegistryUse *use = plan->first_use; use; use = use->next) {
        if (!static_registry_bind(plan, use)) { return false; }
        if (use->key_state == STATIC_REGISTRY_KEY_PENDING) { state = STATIC_REGISTRY_KEY_PENDING; }
    }
    *output = state;
    return true;
}
