/** @file
 * @brief Keep complete typed analysis separate from not-yet-implemented execution adapters.
 * [COMP-02](../../../spec/implementation/compiler.md#comp-02) permits inspecting
 * resolved IR without executing it. [CALL-09](../../../spec/language/calls-and-results.md#call-09),
 * [FLOW-04](../../../spec/language/control-flow.md#flow-04),
 * [FLOW-05](../../../spec/language/lists.md#flow-05) forbid
 * publishing C that drops selected actions or cursor state. Ordinary selected
 * instance trees and self sources have complete runtime/C adapters; intrinsic
 * list operations, header loops and traversals retain their explicit guards.
 */
#include "codegen/internal.h"
/** Retain the exact original site and report unsupported execution before creating any artifact. */
static bool unsupported(NativeScaffold *scaffold, SourceSpan site, const char *message) {
    scaffold_error(scaffold, DIAGNOSTIC_UNSUPPORTED_FEATURE, message, NULL);
    scaffold->error.primary.span = site;
    return false;
}
bool codegen_validate_execution(NativeScaffold *scaffold) {
    for (const SemanticType *type = scaffold->frontend->semantic.first; type != NULL; type = type->next) {
        for (const SemanticStatement *statement = type->statements; statement != NULL; statement = statement->next) {
            const AstNode *node = statement->syntax;
            if (node->kind == AST_FOREACH || (node->kind == AST_LOOP && node->as.control.command != NULL)) {
                return unsupported(scaffold, node->span, "Traversal and condition-chain IR require their execution adapters");
            }
            if (node->kind == AST_CALL && (statement->operation_kind == SEMANTIC_LIST_SERVICE_OPERATION
                    || statement->operation_kind == SEMANTIC_LIST_METHOD_OPERATION)) {
                return unsupported(scaffold, node->span, "Intrinsic list operation IR requires its execution adapter");
            }
        }
    }
    return true;
}
