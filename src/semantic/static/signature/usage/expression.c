/** @file
 * @brief Check one named application without replacing the actual residual map.
 * [TPL-07](../../../../../spec/language/templates.md#tpl-07) requires membership
 * of each supplied name, rather than equality with a use's smaller name set.
 */
#include "../internal.h"

/** Exact foreign-label membership ignores source ordering and local names.
 * [TPL-07](../../../../../spec/language/templates.md#tpl-07) preserves full names.
 */
static const StaticSignatureEntry *member(const StaticSignature *signature, SourceSpan name) {
    for (const StaticSignatureEntry *entry = signature->first; entry != NULL; entry = entry->next) {
        if (static_same_name(entry->name, name)) { return entry; }
    }
    return NULL;
}

StaticUsageResult static_usage_expression(const StaticInventory *inventory,
    const StaticModule *module, const AstTypeExpression *expression,
    StaticCategory category, const StaticSignature *signature, const StaticSymbol *actual_symbol) {
    StaticUsageResult result = {STATIC_USAGE_SATISFIED, {0}, NULL};
    for (const AstStaticBinding *input = expression->first; input != NULL; input = input->next) {
        if (input->kind != AST_STATIC_INPUT) { continue; }
        if (category == STATIC_CATEGORY_TYPE) {
            return (StaticUsageResult){STATIC_USAGE_CONFLICT, input->marker,
                "A Type is not an applied constructor"};
        }
        if (category == STATIC_CATEGORY_DEFERRED || signature == NULL) {
            result.status = STATIC_USAGE_PENDING;
            continue;
        }
        const StaticSignatureEntry *slot = member(signature, input->label);
        if (slot == NULL) {
            return (StaticUsageResult){STATIC_USAGE_CONFLICT, input->label,
                "A named input is absent from the actual residual constructor signature"};
        }
        StaticMetadata argument = static_expression_metadata(inventory, module, input->input);
        if (actual_symbol != NULL && input->input->kind == AST_TYPE_PLACEHOLDER
            && input->input->first == NULL) {
            const StaticSymbol *operand = static_symbol_find(module, input->input->head);
            if (operand->usage_group == actual_symbol->usage_group) { argument.category = category; }
        }
        if (slot->category == STATIC_CATEGORY_DEFERRED || argument.category == STATIC_CATEGORY_DEFERRED) {
            result.status = STATIC_USAGE_PENDING;
        } else if (slot->category != argument.category) {
            return (StaticUsageResult){STATIC_USAGE_CONFLICT, input->input->head,
                "A static input has the wrong Type or Template category"};
        }
    }
    return result;
}
