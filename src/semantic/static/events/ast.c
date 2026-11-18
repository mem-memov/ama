/**
 * @file
 * @brief Collect static source occurrences through owning AST sections and nested bodies.
 *
 * [TPL-02](../../../../spec/language/templates.md#tpl-02) distinguishes interface
 * introduction contexts from shared/body uses. [TPL-12](../../../../spec/language/static-results.md#tpl-12)
 * keeps aliases module-scoped across runtime nesting and excludes foreign formal
 * declarations from local use collection.
 */
#include <assert.h>
#include "internal.h"

/** Inventory-owned block work retains the declaration context for its nodes.
 * Contract: [TPL-02](../../../../spec/language/templates.md#tpl-02).
 */
typedef struct BlockWork {
    struct BlockWork *next;
    const AstNode *node;
    StaticContext context;
} BlockWork;

/** Queue a nonempty owning subtree; alias links are not traversal inputs.
 * Contract: [TPL-12](../../../../spec/language/static-results.md#tpl-12).
 */
static bool push(StaticInventory *inventory, BlockWork **pending, const AstNode *node, StaticContext context) {
    if (node == NULL) { return true; }
    BlockWork *work = static_allocate(inventory, sizeof(*work));
    if (work == NULL) { return false; }
    *work = (BlockWork){*pending, node, context};
    *pending = work;
    return true;
}

/** Each expression retains its actual statement and lexical declaration role.
 * Runtime call targets and foreign static formal/signature names are excluded.
 * Contract: [TPL-02](../../../../spec/language/templates.md#tpl-02),
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12).
 */
static bool node_occurrences(StaticInventory *inventory, StaticModule *module,
    const AstNode *node, StaticContext context) {
    const AstTypeExpression *expression = NULL;
    if (node->kind == AST_DECLARATION) { expression = node->as.declaration.type; }
    else if (node->kind == AST_STATIC_RESULT) {
        expression = node->as.static_result.expression;
        context = STATIC_CONTEXT_DEFINITION;
    } else if (node->kind == AST_STATIC_PROJECTION) {
        expression = node->as.static_projection;
        context = STATIC_CONTEXT_BODY;
    } else if (node->kind == AST_CALL) {
        expression = node->as.call.namespace_expression;
        context = STATIC_CONTEXT_NAMESPACE;
    }
    return static_expression_occurrences(inventory, module, expression, node, context);
}

/** Follow only owning runtime child edges, never condition-owner or argument
 * aliases. All nested body declarations remain static body uses.
 * Contract: [TPL-12](../../../../spec/language/static-results.md#tpl-12).
 */
static bool child_blocks(StaticInventory *inventory, BlockWork **pending, const AstNode *node) {
    if (node->kind == AST_CALL) {
        if (!push(inventory, pending, node->as.call.alternative.first, STATIC_CONTEXT_BODY)) { return false; }
        for (const AstOutputAction *action = node->as.call.first_output; action != NULL; action = action->next) {
            if (action->kind == AST_SELECTED_ACTION
                && !push(inventory, pending, action->operation, STATIC_CONTEXT_BODY)) { return false; }
        }
    } else if (node->kind == AST_BRANCH || node->kind == AST_LOOP) {
        if (!push(inventory, pending, node->as.control.body.first, STATIC_CONTEXT_BODY)
            || !push(inventory, pending, node->as.control.alternative.first, STATIC_CONTEXT_BODY)
            || !push(inventory, pending, node->as.control.command, STATIC_CONTEXT_BODY)) { return false; }
    } else if (node->kind == AST_FOREACH) {
        return push(inventory, pending, node->as.traversal.body.first, STATIC_CONTEXT_BODY);
    }
    return true;
}

/** Collect once into an empty occurrence owner. Every transition consumes one
 * AST owner or removes finite work; no C recursion represents source nesting.
 * Scope/kind lookup is deliberately absent. Caller sorts before classification.
 * Contract: [TPL-02](../../../../spec/language/templates.md#tpl-02),
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12).
 */
bool static_collect_occurrences(StaticInventory *inventory, StaticModule *module) {
    if (inventory->status != STATIC_OK) { return false; }
    assert(module->first_occurrence == NULL && module->occurrence_count == 0);
    const AstModule *syntax = &module->definition->syntax;
    BlockWork *pending = NULL;
    if (!push(inventory, &pending, syntax->constants.first, STATIC_CONTEXT_CONSTANT)
        || !push(inventory, &pending, syntax->parameters.first, STATIC_CONTEXT_INPUT)
        || !push(inventory, &pending, syntax->results.first, STATIC_CONTEXT_RESULT)
        || !push(inventory, &pending, syntax->body.first, STATIC_CONTEXT_BODY)) { return false; }
    while (pending != NULL) {
        BlockWork *work = pending;
        const AstNode *node = work->node;
        if (node == NULL) { pending = work->next; continue; }
        work->node = node->next;
        if (!node_occurrences(inventory, module, node, work->context)
            || !child_blocks(inventory, &pending, node)) { return false; }
    }
    return true;
}
