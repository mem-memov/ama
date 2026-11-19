/** @file
 * @brief Read deterministic static interface identities independently of collector allocation order.
 * [TPL-02](../../../../spec/language/templates.md#tpl-02) fixes input introduction order.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) makes pairs and receipt aliases public outputs.
 */
#include "../inventory.h"

/** Dense group ordinals, not linked-list order, select original interface
 * identities. Receipt-defined aliases are members of the public result group.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) keeps static source binding named, independently of these internal indices.
 */
const StaticSymbol *static_symbol_at(const StaticModule *module, StaticRole role, size_t ordinal) {
    if (module == NULL || (role != STATIC_PARAMETER && role != STATIC_RESULT)) { return NULL; }
    size_t count = role == STATIC_PARAMETER ? module->parameter_count : module->result_count;
    if (ordinal >= count) { return NULL; }
    for (const StaticSymbol *symbol = module->first; symbol; symbol = symbol->next) {
        bool member = (symbol->role == STATIC_PARAMETER) == (role == STATIC_PARAMETER);
        if (member && symbol->ordinal == ordinal) { return symbol; }
    }
    return NULL;
}
