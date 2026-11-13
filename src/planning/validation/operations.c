/** @file
 * @brief Reject typed direct-result trees until explicit selected-target lifetime and action lowering exists.
 * [CALL-09](../../../spec/language/calls-and-results.md#call-09) requires
 * protected parent-result identity and depth-first actions, never caller-local access emission.
 */
#include "planning/internal.h"
bool plan_validate_operations(ProgramPlan *plan) {
    for (const SemanticType *type = plan->semantic->first; type != NULL; type = type->next) {
        for (const SemanticStatement *statement = type->statements; statement != NULL; statement = statement->next) {
            if (statement->syntax->kind == AST_CALL && statement->syntax->as.call.has_selected) {
                plan_fail(plan, statement->syntax->span, "Direct result-selection execution requires explicit target lifetime and action lowering");
                plan->error.code = DIAGNOSTIC_UNSUPPORTED_FEATURE;
                return false;
            }
        }
    }
    return true;
}
