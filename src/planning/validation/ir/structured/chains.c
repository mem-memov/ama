/** @file
 * @brief Check initiating caller/root identities and exact whole-chain refusal continuations.
 * [CALL-09](../../../../../spec/language/calls-and-results.md#call-09) gives
 * every selected link the same initiating caller and outer statement exit while
 * keeping each refused link's handler independent of ancestor handlers.
 */
#include "internal.h"
bool structured_chain(const PlannedType *type, const PlannedChain *candidate) {
    for (const PlannedChain *chain = type->chains; chain != NULL; chain = chain->next) {
        if (chain == candidate) { return true; }
    }
    return false;
}
/** A root must be an exact declared source operation, not a selected child or a forged same-spelled record. */
static bool root(const PlannedType *type, const SemanticStatement *candidate) {
    for (const SemanticStatement *statement = type->type->statements; statement != NULL; statement = statement->next) {
        if (statement == candidate) { return statement->syntax->kind == AST_CALL && statement->enclosing_operation == NULL; }
    }
    return false;
}
bool structured_chains(const PlannedType *type) {
    for (const PlannedChain *chain = type->chains; chain != NULL; chain = chain->next) {
        if (!root(type, chain->root) || chain->root_frame != plan_storage_frame(type, chain->root)
            || !structured_node(type, chain->exit)) { return false; }
        const PlannedLoop *incoming = structured_loop(type, chain->root->enclosing_loop);
        if (chain->break_target != (incoming == NULL ? NULL : incoming->exit)) { return false; }
        if (chain->header != NULL) {
            const PlannedLoop *header = NULL;
            for (const PlannedLoop *loop = type->loops; loop != NULL; loop = loop->next) {
                if (loop == chain->header) { header = loop; break; }
            }
            if (header == NULL || header->header != chain || header->statement->loop_root != chain->root
                || chain->exit != header->after) { return false; }
        }
        for (const PlannedChain *earlier = type->chains; earlier != chain; earlier = earlier->next) {
            if (earlier->root == chain->root) { return false; }
        }
    }
    for (const SemanticStatement *statement = type->type->statements; statement != NULL; statement = statement->next) {
        if (statement->syntax->kind != AST_CALL || statement->enclosing_operation != NULL) { continue; }
        bool found = false;
        for (const PlannedChain *chain = type->chains; chain != NULL; chain = chain->next) {
            if (chain->root == statement) { found = true; break; }
        }
        if (!found) { return false; }
    }
    return true;
}
