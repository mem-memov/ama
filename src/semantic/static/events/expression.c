/**
 * @file
 * @brief Collect static expression heads and receivers without importing foreign labels.
 *
 * [TPL-02](../../../../spec/language/templates.md#tpl-02) separates source
 * expression names from foreign labels; [TPL-12](../../../../spec/language/static-results.md#tpl-12)
 * gives each nested receiver its actual lexical introduction site.
 */
#include "internal.h"

/** Inventory-owned pending expression; root records the caller's type context.
 * Contract: [TPL-02](../../../../spec/language/templates.md#tpl-02).
 */
typedef struct ExpressionWork {
    struct ExpressionWork *next;
    const AstTypeExpression *expression;
    bool root;
} ExpressionWork;

/** Queue a real owner edge, allocating nothing for an absent expression.
 * Contract: [TPL-12](../../../../spec/language/static-results.md#tpl-12).
 */
static bool push(StaticInventory *inventory, ExpressionWork **pending,
    const AstTypeExpression *expression, bool root) {
    if (expression == NULL) { return true; }
    ExpressionWork *work = static_allocate(inventory, sizeof(*work));
    if (work == NULL) { return false; }
    *work = (ExpressionWork){*pending, expression, root};
    *pending = work;
    return true;
}

/** Visit each head/input owner once, retaining raw sites for the subsequent
 * stable lexical sort; do not resolve names or categories during traversal.
 * Contract: [TPL-02](../../../../spec/language/templates.md#tpl-02),
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12).
 */
bool static_expression_occurrences(StaticInventory *inventory, StaticModule *module,
    const AstTypeExpression *expression, const AstNode *statement, StaticContext context) {
    ExpressionWork *pending = NULL;
    if (!push(inventory, &pending, expression, true)) { return false; }
    while (pending != NULL) {
        ExpressionWork *work = pending;
        pending = work->next;
        StaticOccurrence head = {.site = work->expression->head, .expression = work->expression,
            .statement = statement, .context = context, .root = work->root};
        if (!static_occurrence_append(inventory, module, head)) { return false; }
        for (const AstStaticBinding *binding = work->expression->first; binding != NULL; binding = binding->next) {
            if (binding->kind == AST_STATIC_INPUT) {
                if (!push(inventory, &pending, binding->input, false)) { return false; }
            } else {
                StaticOccurrence receiver = head;
                receiver.site = binding->receiver;
                receiver.receiver = binding;
                if (!static_occurrence_append(inventory, module, receiver)) { return false; }
            }
        }
    }
    return true;
}
