/**
 * @file
 * @brief Query exact static module identities and whole-module names.
 *
 * [TPL-02](../../../../spec/language/templates.md#tpl-02) compares complete
 * scoped names. [TPL-12](../../../../spec/language/static-results.md#tpl-12)
 * makes all local input/output names visible throughout their module.
 */
#include <assert.h>
#include <string.h>
#include "../internal.h"

/** Compare original bytes without normalization, prefix matching or allocation.
 * Contract: [TPL-02](../../../../spec/language/templates.md#tpl-02).
 */
bool static_same_name(SourceSpan left, SourceSpan right) {
    assert(source_span_valid(left) && source_span_valid(right));
    size_t length = left.end - left.start;
    return length == right.end - right.start
        && memcmp(source_bytes(left.source) + left.start, source_bytes(right.source) + right.start, length) == 0;
}

/** Module identity is the loaded definition, not a source basename or layout.
 * Contract: [TPL-02](../../../../spec/language/templates.md#tpl-02).
 */
const StaticModule *static_module_find(const StaticInventory *inventory, const ProjectModule *definition) {
    for (const StaticModule *module = inventory->first; module != NULL; module = module->next) {
        if (module->definition == definition) { return module; }
    }
    return NULL;
}

/** Query every collected name throughout the module. Lookup neither changes
 * records nor invents an enclosing/foreign scope or a source-position cutoff.
 * Contract: [TPL-12](../../../../spec/language/static-results.md#tpl-12).
 */
const StaticSymbol *static_symbol_find(const StaticModule *module, SourceSpan name) {
    if (module == NULL) { return NULL; }
    for (const StaticSymbol *symbol = module->first; symbol != NULL; symbol = symbol->next) {
        if (static_same_name(symbol->name, name)) {
            return symbol;
        }
    }
    return NULL;
}
