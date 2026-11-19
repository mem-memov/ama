/** @file
 * @brief Assign deterministic source-defined record indices within static input or public output groups.
 * [TPL-02](../../../../spec/language/templates.md#tpl-02) orders first input introductions.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) makes pair/receipt outputs public;
 * these internal indices do not create positional static binding syntax.
 */
#include <assert.h>
#include "../internal.h"

/** Rank distinct defining offsets in one complete group, without allocation or
 * changing ownership links. Source collection guarantees distinct introductions;
 * a repeated receipt reuses a symbol and never becomes another ranked item.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) preserves definition identity.
 */
void static_order_symbols(StaticModule *module, StaticRole group) {
    assert(group == STATIC_PARAMETER || group == STATIC_RESULT);
    bool parameters = group == STATIC_PARAMETER;
    for (StaticSymbol *symbol = module->first; symbol != NULL; symbol = symbol->next) {
        if ((symbol->role == STATIC_PARAMETER) != parameters) { continue; }
        size_t ordinal = 0;
        for (const StaticSymbol *other = module->first; other != NULL; other = other->next) {
            if ((other->role == STATIC_PARAMETER) == parameters && other->site.start < symbol->site.start) {
                ordinal++;
            }
        }
        symbol->ordinal = ordinal;
    }
}
