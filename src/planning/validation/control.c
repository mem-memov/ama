/** @file
 * @brief Stop typed traversal and command-header loops before incomplete lexical frames or restart edges enter old lowering.
 * [FLOW-04](../../../spec/language/control-flow.md#flow-04) and
 * [FLOW-05](../../../spec/language/lists.md#flow-05) require explicit full-header
 * restart and live-boundary traversal frames, not existing caller-field emission.
 */
#include "planning/internal.h"
bool plan_validate_control(ProgramPlan *plan) {
    for (const SemanticType *type = plan->semantic->first; type != NULL; type = type->next) {
        for (const SemanticStatement *statement = type->statements; statement != NULL; statement = statement->next) {
            const AstNode *node = statement->syntax;
            if (node->kind == AST_FOREACH || (node->kind == AST_LOOP && node->as.control.command != NULL)) {
                plan_fail(plan, node->span, "Traversal and condition-chain execution require explicit lexical frames and restart lowering");
                plan->error.code = DIAGNOSTIC_UNSUPPORTED_FEATURE;
                return false;
            }
        }
    }
    return true;
}
