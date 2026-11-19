/** @file
 * @brief Form finite exact classes of bare local result-forwarding equalities.
 * [TPL-07](../../../../../spec/language/templates.md#tpl-07) derives requirements
 * through every forwarding path. Applied results and producer equations remain
 * distinct obligations for their normalizers, rather than guessed equalities.
 */
#include "../internal.h"

void static_prepare_usage_groups(StaticInventory *inventory) {
    for (StaticModule *module = inventory->first; module != NULL; module = module->next) {
        for (StaticSymbol *symbol = module->first; symbol != NULL; symbol = symbol->next) {
            symbol->usage_group = symbol;
        }
        if (module->definition->syntax.sections.kind != MODULE_STATIC) { continue; }
        for (StaticSymbol *symbol = module->first; symbol != NULL; symbol = symbol->next) {
            if (symbol->role != STATIC_RESULT) { continue; }
            const AstTypeExpression *value = symbol->declaration->as.static_result.expression;
            if (value->kind != AST_TYPE_PLACEHOLDER || value->first != NULL) { continue; }
            const StaticSymbol *source = static_symbol_find(module, value->head);
            const StaticSymbol *left = symbol->usage_group, *right = source->usage_group;
            if (left == right) { continue; }
            const StaticSymbol *representative = left->site.start < right->site.start ? left : right;
            for (StaticSymbol *member = module->first; member != NULL; member = member->next) {
                if (member->usage_group == left || member->usage_group == right) {
                    member->usage_group = representative;
                }
            }
        }
    }
}
