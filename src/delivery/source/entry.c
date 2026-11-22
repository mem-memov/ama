/** @file
 * @brief Distinguish independently concrete tests from genuinely pending generic roots.
 * [PKG-08](../../../spec/project/packages.md#pkg-08) retains open helpers;
 * [TPL-03](../../../spec/language/templates.md#tpl-03) allows typed defaults and
 * other internal equations to close originally parameterized definitions.
 * [TPL-11](../../../spec/language/templates.md#tpl-11) identifies the actual root
 * use so unrelated unresolved fields do not become helper classification.
 */
#include "entry.h"
DeliveryEntryState delivery_source_entry_state(const CompilerFrontend *frontend) {
    if (!frontend || !frontend->attempted || frontend->authoring || !frontend->project.entry) { return DELIVERY_ENTRY_INVALID; }
    if (frontend->valid && frontend->plan_initialized) { return DELIVERY_ENTRY_EXECUTABLE; }
    if (!frontend->specialization_initialized || frontend->specializations.closure.status != STATIC_CLOSURE_UNRESOLVED) {
        return DELIVERY_ENTRY_INVALID;
    }
    const StaticModule *module = static_module_find(&frontend->static_inventory, frontend->project.entry);
    if (!module || !module->parameter_count) { return DELIVERY_ENTRY_INVALID; }
    for (const StaticRegistryUse *use = frontend->specializations.registry.first_use; use; use = use->next) {
        const StaticCreationPath *path = use->path;
        if (!path->parent && !path->required_from && !path->application && !path->source_only &&
            path->owner->environment->definition == module && !path->owner->environment->owner_type &&
            use->key_state == STATIC_REGISTRY_KEY_PENDING && !use->candidate) { return DELIVERY_ENTRY_OPEN; }
    }
    return DELIVERY_ENTRY_INVALID;
}
