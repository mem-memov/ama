/** @file
 * @brief Collect first receipt definitions after every static graph pair is already known.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) gives pairs priority,
 * makes all receivers public and distinguishes first definitions from equality receipts.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) retains failed partial records.
 */
#include "../internal.h"

/** Visit producer roots and source-ordered output edges only. Values have no
 * nested receipts under the grammar; foreign labels are never local binders.
 * Existing pair/alias identities are immutable. Count/allocation refusal leaves
 * every completed prefix inventory-owned and prevents successful publication.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) fixes roles before solving.
 */
bool static_collect_receivers(StaticInventory *inventory, StaticModule *module) {
    if (inventory->status != STATIC_OK) { return false; }
    for (const AstNode *node = module->definition->syntax.graph.first; node != NULL; node = node->next) {
        if (node->kind != AST_STATIC_PROJECTION) { continue; }
        for (const AstStaticBinding *binding = node->as.static_projection->first;
            binding != NULL; binding = binding->next) {
            if (binding->kind != AST_STATIC_OUTPUT) { continue; }
            if (static_symbol_find(module, binding->receiver) != NULL) { continue; }
            StaticSymbol *symbol = static_symbol_add(inventory, module, binding->receiver,
                binding->receiver, STATIC_ALIAS, STATIC_CATEGORY_DEFERRED);
            if (symbol == NULL) { return false; }
            symbol->declaration = node;
            symbol->receiver = binding;
        }
    }
    return true;
}
