/** @file
 * @brief Lower fixed list interfaces into typed staging/preflight/prepare/commit, not module instances.
 * [LIST-04](../../../../spec/language/lists.md#list-04) gives immutable typed
 * method descriptors; [LIST-10](../../../../spec/language/lists.md#list-10)
 * permits refusal only before mutation after required inputs have been staged.
 */
#include "lowering/calls/internal.h"
#include "lowering/replacement/internal.h"
#include "semantic/lists/interface.h"
/** Match a supplied method input by its checked ordinal; omissions select the fixed input type's default. */
static const SemanticBinding *input(const SemanticStatement *statement, size_t position) {
    for (const SemanticBinding *binding = statement->bindings; binding != NULL; binding = binding->next) {
        if (binding->syntax->kind == AST_INPUT && binding->position == position && !binding->syntax->omitted) { return binding; }
    }
    return NULL;
}
ControlNode *lower_list_method(Lowering *lowering, const SemanticStatement *statement,
    ControlNode *outputs, ControlNode *refusal) {
    ControlNode *commit = lower_prepend(lowering, CONTROL_LIST_METHOD_COMMIT, statement, NULL, outputs);
    ControlNode *prepare = commit == NULL ? NULL : lower_prepend(lowering, CONTROL_LIST_METHOD_PREPARE, statement, NULL, commit);
    ControlNode *preflight = prepare == NULL ? NULL : lower_prepend(lowering, CONTROL_LIST_METHOD_PREFLIGHT, statement, NULL, prepare);
    if (preflight == NULL) { return NULL; }
    commit->chain = prepare->chain = preflight->chain = lowering->chain;
    preflight->alternative = refusal;
    size_t count = 0;
    while (semantic_list_position(statement->list_method, false, count) != NULL) { count++; }
    ControlNode *next = preflight;
    for (; count > 0; count--) {
        size_t position = count - 1;
        next = lower_method_parameter(lowering, statement,
            semantic_list_position(statement->list_method, false, position), position, input(statement, position), next);
        if (next == NULL) { return NULL; }
    }
    ReverseBinding *reverse = lower_reverse_bindings(lowering, statement);
    if (lowering->plan->status != PLAN_OK) { return NULL; }
    return lower_inputs(lowering, statement, reverse, next);
}
