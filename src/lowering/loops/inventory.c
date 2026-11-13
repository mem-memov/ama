/** @file
 * @brief Allocate exact lexical loop identities before patching any loop or body successor.
 * [FLOW-04](../../../spec/language/control-flow.md#flow-04) fixes each loop's
 * own repeat root; [FLOW-05](../../../spec/language/lists.md#flow-05) associates
 * traversal state with that source site rather than a dynamic execution history.
 */
#include "lowering/control/internal.h"
#include "planning/storage/frames/internal.h"
PlannedLoop *lower_loop(const PlannedType *type, const SemanticStatement *statement) {
    if (statement == NULL) { return NULL; }
    for (PlannedLoop *loop = type->loops; loop != NULL; loop = loop->next) {
        if (loop->statement == statement) { return loop; }
    }
    return NULL;
}
bool lower_loop_inventory(Lowering *lowering) {
    for (const SemanticStatement *statement = lowering->type->type->statements; statement != NULL; statement = statement->next) {
        if (statement->syntax->kind != AST_LOOP && statement->syntax->kind != AST_FOREACH) { continue; }
        PlannedLoop *loop = plan_allocate(lowering->plan, sizeof(*loop));
        if (loop == NULL) { return false; }
        *loop = (PlannedLoop){.statement = statement, .frame = plan_storage_frame(lowering->type, statement)};
        if (lowering->type->last_loop == NULL) { lowering->type->loops = loop; }
        else { lowering->type->last_loop->next = loop; }
        lowering->type->last_loop = loop;
    }
    for (PlannedLoop *loop = lowering->type->loops; loop != NULL; loop = loop->next) {
        loop->parent = lower_loop(lowering->type, loop->statement->enclosing_loop);
    }
    return true;
}
