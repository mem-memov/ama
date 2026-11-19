/** @file
 * @brief Intern explicit lexical environments without unfolding their definitions.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) distinguishes owners.
 */
#include <stdint.h>
#include "internal.h"

/** Validate borrowed identities before publishing one immutable environment.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) preserves owner identity.
 */
bool static_term_environment(StaticTermStore *store, const StaticModule *definition,
    const StaticTerm *owner, const StaticTermEnvironment **output) {
    if (store->status != STATIC_TERM_OK) { return false; }
    if (output == NULL || !static_term_has_module(store, definition) ||
        (owner != NULL && (!static_term_has_value(store, owner) ||
        owner->kind != STATIC_TERM_NOMINAL || owner->category != STATIC_CATEGORY_TYPE ||
        owner->as.nominal.origin != definition))) {
        return static_term_invalid(store, (SourceSpan){0}, "Invalid static environment identity");
    }
    for (StaticTermEnvironment *item = store->first_environment; item != NULL; item = item->next) {
        if (item->definition == definition && item->owner_type == owner && !item->producer_applied) {
            *output = item;
            return true;
        }
    }
    if (store->environment_count == SIZE_MAX) { return static_term_memory_failure(store); }
    StaticTermEnvironment *item = static_term_allocate(store, sizeof(*item));
    if (item == NULL) { return false; }
    item->definition = definition;
    item->owner_type = owner;
    if (store->last_environment != NULL) { store->last_environment->next = item; }
    else { store->first_environment = item; }
    store->last_environment = item;
    store->environment_count++;
    *output = item;
    return true;
}
