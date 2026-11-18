/**
 * @file
 * @brief Orchestrate one whole-project static lexical inventory before later application inference.
 *
 * [TPL-02](../../../../spec/language/templates.md#tpl-02) requires whole-
 * interface collection before uses. [TPL-12](../../../../spec/language/static-results.md#tpl-12)
 * establishes aliases lexically. [TPL-07](../../../../spec/language/templates.md#tpl-07)
 * validates known signatures while preserving pending metadata obligations.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) publishes validity
 * only after complete success and leaves failed construction owned.
 */
#include <assert.h>
#include "../internal.h"
#include "../classify/sites.h"

/** One attempt collects all definitions before inspecting cross-module category
 * metadata. Validity certifies scope/known signatures, never application closure,
 * result equations, specialization legality or executable integration.
 * Contract: [TPL-02](../../../../spec/language/templates.md#tpl-02).
 */
StaticStatus static_inventory_collect(StaticInventory *inventory) {
    assert(!inventory->attempted && inventory->first == NULL);
    inventory->attempted = true;
    for (const ProjectModule *definition = inventory->project->first; definition != NULL; definition = definition->next) {
        StaticModule *module = static_module_add(inventory, definition);
        if (module == NULL || !static_collect_explicit(inventory, module)
            || !static_collect_occurrences(inventory, module)) { return inventory->status; }
        static_sort_occurrences(module);
        if (!static_classify_names(inventory, module)
            || !static_bind_occurrences(inventory, module)) { return inventory->status; }
    }
    if (!static_prepare_signatures(inventory) || !static_check_signatures(inventory)) { return inventory->status; }
    inventory->valid = true;
    return STATIC_OK;
}
