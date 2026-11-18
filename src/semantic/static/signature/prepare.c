/**
 * @file
 * @brief Build constructor and input-pattern signature skeletons before resolving dependent categories.
 *
 * [TPL-07](../../../../spec/language/templates.md#tpl-07) fixes finite named
 * signatures independently of actual values. [TPL-02](../../../../spec/language/templates.md#tpl-02)
 * preserves owner parameter order separately from foreign signature name sets.
 */
#include <stdint.h>
#include "signature.h"
#include "../internal.h"
#include "../classify/sites.h"

/** Append a prepared foreign slot, rejecting exact duplicate labels and count
 * overflow before publication. Its expression remains an explicit obligation.
 * Component contract: [TPL-07](../../../../spec/language/templates.md#tpl-07).
 */
static bool append(StaticInventory *inventory, StaticSignature *signature, SourceSpan name,
    const AstTypeExpression *expression) {
    for (const StaticSignatureEntry *prior = signature->first; prior != NULL; prior = prior->next) {
        if (static_same_name(prior->name, name)) {
            return static_invalid(inventory, name, "Duplicate constructor pattern slot", NULL);
        }
    }
    if (signature->count == SIZE_MAX) { return static_memory_failure(inventory); }
    StaticSignatureEntry *entry = static_allocate(inventory, sizeof(*entry));
    if (entry == NULL) { return false; }
    entry->name = name;
    entry->pending_expression = expression;
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
        for (const StaticSymbol *symbol = module->first; symbol != NULL; symbol = symbol->next) {
            if (symbol->role == STATIC_PARAMETER && symbol->ordinal == ordinal) {
                if (!append(inventory, module->constructor_signature, symbol->name, NULL)) { return false; }
                break;
            }
        }
    }
    return true;
}

/** Preserve every named root-input pattern for later agreement checking. Its
 * slot categories depend on local static expressions, not foreign label text.
 * Component contract: [TPL-07](../../../../spec/language/templates.md#tpl-07).
 */
static bool pattern(StaticInventory *inventory, StaticModule *module, const StaticOccurrence *site) {
    StaticPattern *pattern = static_allocate(inventory, sizeof(*pattern));
    if (pattern == NULL) { return false; }
    pattern->signature = static_allocate(inventory, sizeof(*pattern->signature));
    if (pattern->signature == NULL) { return false; }
    pattern->occurrence = site;
    for (const AstStaticBinding *binding = site->expression->first; binding != NULL; binding = binding->next) {
        if (binding->kind != AST_STATIC_INPUT) { pattern->output_phase = true; continue; }
        if (binding->positional) {
            return static_invalid(inventory, binding->marker, "A named input pattern cannot mix positional inputs", NULL);
        }
        if (!append(inventory, pattern->signature, binding->label, binding->input)) { return false; }
    }
    if (module->last_pattern == NULL) { module->first_pattern = pattern; }
    else { module->last_pattern->next = pattern; }
    module->last_pattern = pattern;
    return true;
}

/** Build independent finite trees for explicit declarations first, then pattern
 * skeletons and constructor interfaces across all modules before resolving any
 * cross-module category. Partial allocation remains inventory-owned.
 * Contract: [TPL-07](../../../../spec/language/templates.md#tpl-07).
 */
bool static_prepare_signatures(StaticInventory *inventory) {
    if (inventory->status != STATIC_OK) { return false; }
    for (StaticModule *module = inventory->first; module != NULL; module = module->next) {
        for (StaticSymbol *symbol = module->first; symbol != NULL; symbol = symbol->next) {
            if (symbol->role == STATIC_PARAMETER && symbol->category == STATIC_CATEGORY_TEMPLATE
                && symbol->declaration->kind == AST_STATIC_FORMAL) {
                if (!static_signature_build(inventory, symbol->declaration->as.static_formal->first,
                    &symbol->signature)) { return false; }
            }
        }
        for (const StaticOccurrence *site = module->first_occurrence; site != NULL; site = site->next) {
            if (static_named_input_pattern(site)) {
                if (!pattern(inventory, module, site)) { return false; }
            }
        }
        for (StaticSymbol *symbol = module->first; symbol != NULL; symbol = symbol->next) {
            if (symbol->role != STATIC_PARAMETER || symbol->category != STATIC_CATEGORY_TEMPLATE || symbol->signature != NULL) { continue; }
            for (StaticPattern *item = module->first_pattern; item != NULL; item = item->next) {
                if (item->occurrence->symbol == symbol) { symbol->signature = item->signature; break; }
            }
        }
        if (!constructor(inventory, module)) { return false; }
    }
    return true;
}
