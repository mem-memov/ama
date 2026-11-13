/** @file
 * @brief Complete condition and traversal metadata after source-ordered header operation resolution.
 * [FLOW-04](../../../spec/language/control-flow.md#flow-04) needs the full
 * header inventory; [SYN-01](../../../spec/language/source-and-names.md#syn-01)
 * defines empty bodies by syntax rather than emitted work or runtime contents.
 */
#include "internal.h"
bool semantic_controls(SemanticProgram *program, SemanticType *type) {
    for (SemanticStatement *statement = type->statements; statement != NULL; statement = statement->next) {
        const AstNode *node = statement->syntax;
        if (node->kind == AST_BRANCH || node->kind == AST_LOOP) {
            if (!semantic_control_condition(program, type, statement)) { return false; }
            statement->negative_branch = node->kind == AST_BRANCH && node->as.control.body_marker.source == NULL
                && node->as.control.alternative_marker.source != NULL;
            statement->empty_body = statement->negative_branch ? node->as.control.alternative.first == NULL
                : node->as.control.body.first == NULL;
        } else if (node->kind == AST_FOREACH) {
            if (!semantic_control_traversal(program, type, statement)) { return false; }
            statement->empty_body = node->as.traversal.body.first == NULL;
        }
    }
    return true;
}
