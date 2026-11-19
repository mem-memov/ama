/**
 * @file
 * @brief Build actual ordinary constructor signature skeletons before metadata propagation.
 *
 * [TPL-07](../../../../spec/language/templates.md#tpl-07) fixes finite named
 * signatures independently of actual values. [TPL-02](../../../../spec/language/templates.md#tpl-02)
 * preserves owner parameter order separately from foreign signature name sets.
 */
#include <assert.h>
#include <stdint.h>
#include "signature.h"
#include "../internal.h"
#include "../classify/sites.h"

/** Append a prepared foreign slot, rejecting exact duplicate labels and count
 * overflow before publication. Its category comes from the owning formal.
 * Component contract: [TPL-07](../../../../spec/language/templates.md#tpl-07).
 */
static bool append(StaticInventory *inventory, StaticSignature *signature, SourceSpan name) {
    for (const StaticSignatureEntry *prior = signature->first; prior != NULL; prior = prior->next) {
        if (static_same_name(prior->name, name)) {
            return static_invalid(inventory, name, "Duplicate constructor pattern slot", NULL);
        }
    }
    if (signature->count == SIZE_MAX) { return static_memory_failure(inventory); }
    StaticSignatureEntry *entry = static_allocate(inventory, sizeof(*entry));
    if (entry == NULL) { return false; }
    entry->name = name;
    if (signature->last == NULL) { signature->first = entry; }
    else { signature->last->next = entry; }
    signature->last = entry;
    signature->count++;
    return true;
}

/** Constructor slots are the actual owner formals, in original ordinal order;
 * result binders, aliases and foreign signature members never become slots.
 * Component contract: [TPL-07](../../../../spec/language/templates.md#tpl-07).
 */
static bool constructor(StaticInventory *inventory, StaticModule *module) {
    if (module->parameter_count == 0) { return true; }
    module->constructor_signature = static_allocate(inventory, sizeof(*module->constructor_signature));
    if (module->constructor_signature == NULL) { return false; }
    for (size_t ordinal = 0; ordinal < module->parameter_count; ordinal++) {
        const StaticSymbol *symbol = static_symbol_at(module, STATIC_PARAMETER, ordinal);
        assert(symbol != NULL);
        if (!append(inventory, module->constructor_signature, symbol->name)) { return false; }
    }
    return true;
}

/** Build actual ordinary constructor interfaces from their Type formals only.
 * Graph minimum requirements have a separate later collector; there are no
 * expected-signature AST trees under [TPL-07](../../../../spec/language/templates.md#tpl-07).
 */
bool static_prepare_signatures(StaticInventory *inventory) {
    if (inventory->status != STATIC_OK) { return false; }
    for (StaticModule *module = inventory->first; module != NULL; module = module->next) {
        if (!constructor(inventory, module)) { return false; }
    }
    return true;
}
