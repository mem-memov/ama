/**
 * @file
 * @brief Insert distinct static definitions and local names with stable ownership.
 *
 * [TPL-02](../../../../spec/language/templates.md#tpl-02) requires one identity
 * per local name and rejects duplicate explicit binders.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) keeps result/alias
 * records outside the free-parameter count; visibility never permits shadowing.
 */
#include <assert.h>
#include <stdint.h>
#include "../internal.h"

/** Reuse exact definition identity, otherwise append one fully prepared owner.
 * Contract: [TPL-02](../../../../spec/language/templates.md#tpl-02).
 * Allocation refusal leaves the existing module chain unchanged.
 */
StaticModule *static_module_add(StaticInventory *inventory, const ProjectModule *definition) {
    assert(definition != NULL);
    if (inventory->status != STATIC_OK) { return NULL; }
    for (StaticModule *module = inventory->first; module != NULL; module = module->next) {
        if (module->definition == definition) { return module; }
    }
    StaticModule *module = static_allocate(inventory, sizeof(*module));
    if (module == NULL) { return NULL; }
    module->definition = definition;
    if (inventory->last == NULL) { inventory->first = module; }
    else { inventory->last->next = module; }
    inventory->last = module;
    return module;
}

/** Test uniqueness before allocation and link only a fully prepared symbol.
 * Later alias equality operations must find/reuse the existing symbol instead
 * of calling insertion. Contract: [TPL-02](../../../../spec/language/templates.md#tpl-02),
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12).
 */
StaticSymbol *static_symbol_add(StaticInventory *inventory, StaticModule *module,
    SourceSpan name, SourceSpan site, StaticRole role, StaticCategory category) {
    assert(module != NULL && source_span_valid(name) && source_span_valid(site));
    assert(role >= STATIC_PARAMETER && role <= STATIC_ALIAS);
    if (inventory->status != STATIC_OK) { return NULL; }
    for (StaticSymbol *symbol = module->first; symbol != NULL; symbol = symbol->next) {
        if (static_same_name(symbol->name, name)) {
            (void)static_invalid(inventory, site, "The static name is already declared in this module", symbol);
            return NULL;
        }
    }
    size_t *count = role == STATIC_PARAMETER ? &module->parameter_count : &module->result_count;
    if (*count == SIZE_MAX) { (void)static_memory_failure(inventory); return NULL; }
    StaticSymbol *symbol = static_allocate(inventory, sizeof(*symbol));
    if (symbol == NULL) { return NULL; }
    *symbol = (StaticSymbol){.owner = module, .name = name, .site = site,
        .role = role, .category = category, .ordinal = SIZE_MAX};
    if (module->last == NULL) { module->first = symbol; }
    else { module->last->next = symbol; }
    module->last = symbol;
    (*count)++;
    return symbol;
}
