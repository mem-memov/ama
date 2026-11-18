/** @file
 * @brief Read original static interface positions independently of collector allocation order.
 * [TPL-02](../../../../spec/language/templates.md#tpl-02) fixes input introduction order.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) separates explicit result positions.
 */
#include "../inventory.h"

/** Dense role ordinals, not linked-list position or names, select original
 * interface identities. Aliases have no position in either static sequence.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) excludes aliases from positions.
 */
const StaticSymbol *static_symbol_at(const StaticModule *module, StaticRole role, size_t ordinal) {
    if (module == NULL || (role != STATIC_PARAMETER && role != STATIC_RESULT)) { return NULL; }
    size_t count = role == STATIC_PARAMETER ? module->parameter_count : module->result_count;
    if (ordinal >= count) { return NULL; }
    for (const StaticSymbol *symbol = module->first; symbol; symbol = symbol->next) {
        if (symbol->role == role && symbol->ordinal == ordinal) { return symbol; }
    }
    return NULL;
}
