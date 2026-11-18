/**
 * @file
 * @brief Validate exact source/environment/value identities without dereferencing foreign records.
 *
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) prevents same-name
 * capture; already accepted owners, not spelling/layout, establish membership.
 */
#include "../internal.h"

/** Only modules published by the borrowed accepted inventory belong here.
 * Contract: [TPL-08](../../../../../spec/language/templates.md#tpl-08).
 */
bool static_term_has_module(const StaticTermStore *store, const StaticModule *definition) {
    for (const StaticModule *module = store->inventory->first; module != NULL; module = module->next) {
        if (module == definition) { return true; }
    }
    return false;
}

/** Compare symbol pointers before accessing caller-supplied symbol metadata.
 * Contract: [TPL-08](../../../../../spec/language/templates.md#tpl-08).
 */
bool static_term_has_symbol(const StaticTermStore *store, const StaticSymbol *symbol) {
    for (const StaticModule *module = store->inventory->first; module != NULL; module = module->next) {
        for (const StaticSymbol *candidate = module->first; candidate != NULL; candidate = candidate->next) {
            if (candidate == symbol) { return true; }
        }
    }
    return false;
}

/** Environment identity is the store's published record, not a copied shape.
 * Contract: [TPL-08](../../../../../spec/language/templates.md#tpl-08).
 */
bool static_term_has_environment(const StaticTermStore *store, const StaticTermEnvironment *environment) {
    for (const StaticTermEnvironment *candidate = store->first_environment; candidate != NULL; candidate = candidate->next) {
        if (candidate == environment) { return true; }
    }
    return false;
}

/** Values are borrowed only after exact publication membership succeeds.
 * Contract: [TPL-08](../../../../../spec/language/templates.md#tpl-08).
 */
bool static_term_has_value(const StaticTermStore *store, const StaticTerm *value) {
    for (const StaticTerm *candidate = store->first; candidate != NULL; candidate = candidate->next) {
        if (candidate == value) { return true; }
    }
    return false;
}
