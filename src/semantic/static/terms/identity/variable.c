/** @file
 * @brief Intern scoped variable identities independently of future equation solutions.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) fixes lexical identity.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial vertices until store destruction.
 */
#include <stdint.h>
#include "../internal.h"

/** Compare every identity component, never struct padding or spelling.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) separates occurrences.
 */
static bool same_identity(StaticVariableIdentity left, StaticVariableIdentity right) {
    return left.role == right.role && left.environment == right.environment &&
        left.symbol == right.symbol && left.declaration == right.declaration && left.producer == right.producer &&
        left.runtime_access == right.runtime_access;
}

/** Publish only complete vertices; prior records survive any allocation refusal.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial storage.
 */
bool static_term_variable(StaticTermStore *store, StaticVariableIdentity identity,
    StaticCategory category, const StaticSignature *signature, SourceSpan site, const StaticTerm **output) {
    for (StaticTerm *item = store->first; item != NULL; item = item->next) {
        if (item->kind == STATIC_TERM_VARIABLE && same_identity(item->as.variable, identity)) {
            *output = item;
            return true;
        }
    }
    if (store->term_count == SIZE_MAX) { return static_term_memory_failure(store); }
    StaticTerm *item = static_term_allocate(store, sizeof(*item));
    if (item == NULL) { return false; }
    item->owner = store;
    item->ordinal = store->term_count;
    item->kind = STATIC_TERM_VARIABLE;
    item->category = category;
    item->signature = signature;
    item->site = site;
    item->as.variable = identity;
    if (store->last != NULL) { store->last->next = item; }
    else { store->first = item; }
    store->last = item;
    store->term_count++;
    *output = item;
    return true;
}
