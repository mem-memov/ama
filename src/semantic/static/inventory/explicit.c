/**
 * @file
 * @brief Collect all static graph pair and receipt binders before free-input classification.
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
    if (node->kind != AST_STATIC_RESULT) { return true; }
    SourceSpan name = node->as.static_result.name;
    StaticSymbol *symbol = static_symbol_add(inventory, module, name, name,
        STATIC_RESULT, STATIC_CATEGORY_DEFERRED);
    if (symbol == NULL) { return false; }
    symbol->declaration = node;
    return true;
}

/** Complete every pair before classifying receivers, then assign public indices.
 * Ordinary fields and nested foreign labels add no public graph result.
 * Must precede implicit symbol insertion; partial records remain owned on error.
 * Contract: [TPL-02](../../../../spec/language/templates.md#tpl-02),
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12).
 */
bool static_collect_explicit(StaticInventory *inventory, StaticModule *module) {
    if (inventory->status != STATIC_OK) { return false; }
    assert(module->first == NULL && module->parameter_count == 0 && module->result_count == 0);
    for (const AstNode *node = module->definition->syntax.graph.first; node != NULL; node = node->next) {
        if (!declaration(inventory, module, node)) { return false; }
    }
    if (!static_collect_receivers(inventory, module)) { return false; }
    static_order_symbols(module, STATIC_RESULT);
    return true;
}
