/**
 * @file
 * @brief Collect named static graph results before free-input classification.
 *
 * [TPL-02](../../../../spec/language/templates.md#tpl-02) gives graph binders
 * whole-module scope. [TPL-07](../../../../spec/language/templates.md#tpl-07)
 * separates deferred Type/Template metadata from source binders; foreign labels
 * are not owner formals. [TPL-12](../../../../spec/language/static-results.md#tpl-12)
 * gives static results their own textual interface order and no free input slot.
 */
#include <assert.h>
#include "../internal.h"

/** Retain one declaration's exact header and AST provenance; prepare a symbol
 * before adding any metadata. Semantic signatures are built in the next phase,
 * not guessed from uses. Contract: [TPL-02](../../../../spec/language/templates.md#tpl-02),
 * [TPL-07](../../../../spec/language/templates.md#tpl-07),
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12).
 */
static bool declaration(StaticInventory *inventory, StaticModule *module, const AstNode *node) {
    SourceSpan name;
    StaticRole role;
    StaticCategory category;
    if (node->kind == AST_STATIC_RESULT) {
        name = node->as.static_result.name;
        role = STATIC_RESULT;
        category = STATIC_CATEGORY_DEFERRED;
    } else { return true; }
    StaticSymbol *symbol = static_symbol_add(inventory, module, name, name, role, category);
    if (symbol == NULL) { return false; }
    symbol->declaration = node;
    if (role == STATIC_RESULT) { symbol->ordinal = module->result_count - 1; }
    return true;
}

/** Consume each public graph pair once. Ordinary fields and nested foreign
 * labels add no public graph result.
 * Must precede implicit symbol insertion; partial records remain owned on error.
 * Contract: [TPL-02](../../../../spec/language/templates.md#tpl-02),
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12).
 */
bool static_collect_explicit(StaticInventory *inventory, StaticModule *module) {
    if (inventory->status != STATIC_OK) { return false; }
    assert(module->first == NULL && module->parameter_count == 0 && module->result_count == 0);
    const AstModule *syntax = &module->definition->syntax;
    AstBlock blocks[] = {syntax->graph};
    for (size_t block = 0; block < sizeof(blocks) / sizeof(*blocks); block++) {
        for (const AstNode *node = blocks[block].first; node != NULL; node = node->next) {
            if (!declaration(inventory, module, node)) { return false; }
        }
    }
    return true;
}
