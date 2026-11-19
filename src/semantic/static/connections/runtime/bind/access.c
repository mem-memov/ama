/** @file
 * @brief Bind original caller accesses and index operands without requiring concrete Types.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) preserves caller-side index occurrences.
 * [NAME-06](../../../../../../spec/language/declarations.md#name-06) forbids construction-time local access.
 */
#include "../../internal.h"

/** Known storage form is checked now; index Type/capability remains a later
 * typed obligation. No suffix is inserted or inferred away. Each index link is
 * original finite AST structure, so deep indices need no recursive host call.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) separates lexical/storage facts and Type equations.
 */
bool static_runtime_access(StaticConnectionPlan *plan, StaticRuntimeModule *module,
    const StaticRuntimeEvent *event, const AstAccess *access, bool index, const StaticRuntimeField **output) {
    const StaticRuntimeField *initializing = event->kind == STATIC_RUNTIME_INITIALIZER ?
        static_runtime_declaration(module, event->node) : NULL;
    if (access == NULL || (event->kind == STATIC_RUNTIME_INITIALIZER && initializing == NULL)) {
        return static_connection_fail(plan, STATIC_CONNECTION_INVALID, event->site, "Runtime access lacks its original operand or initializer declaration");
    }
    const StaticRuntimeField *first = NULL;
    for (const AstAccess *node = access; node; node = node->index) {
        const StaticRuntimeField *field;
        StaticRuntimeUse use = {.statement = event->node, .syntax = node, .site = node->span,
            .initializing = initializing, .index = index};
        if (!static_runtime_use(plan, module, use, &field)) { return false; }
        if (field->reference != node->reference || field->list != (node->whole_list || node->index != NULL)) {
            return static_connection_fail(plan, STATIC_CONNECTION_CONFLICT, node->span, "Runtime access storage suffix differs from its declaration");
        }
        if (index && (field->reference || node->whole_list)) {
            return static_connection_fail(plan, STATIC_CONNECTION_CONFLICT, node->span, "Index operands require owned scalar storage");
        }
        if (first == NULL) { first = field; }
        index = true;
    }
    *output = first;
    return true;
}
