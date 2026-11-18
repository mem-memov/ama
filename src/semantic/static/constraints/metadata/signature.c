/** @file
 * @brief Cache exact immutable signature-pair outcomes without discarding Pending obligations.
 * [TPL-07](../../../../../spec/language/templates.md#tpl-07) requires recursive equality.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) shares finite structural work.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) releases per-comparison scratch immediately.
 */
#include <stdint.h>
#include "../internal.h"

/** Recursive signatures are immutable; independently resolving metadata chooses
 * a new description pointer, so cached results require no guessed invalidation.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) preserves exact facts.
 */
bool static_constraint_signature(StaticConstraintSolver *solver, const StaticSignature *left,
    const StaticSignature *right, SourceSpan site, StaticSignatureComparison *output) {
    for (const StaticConstraintSignature *item = solver->first_signature; item; item = item->next) {
        if ((item->left == left && item->right == right) || (item->left == right && item->right == left)) {
            *output = item->result;
            return true;
        }
    }
    if (solver->signature_count == SIZE_MAX) {
        return static_constraint_failure(solver, STATIC_CONSTRAINT_OUT_OF_MEMORY, site,
            "Static signature comparison count exceeds representable storage", NULL, NULL);
    }
    StaticSignatureComparison result;
    SourceArena scratch;
    source_arena_init(&scratch, solver->arena.allocator);
    bool success = static_signature_compare_arena(&scratch, left, right, STATIC_SIGNATURE_EXACT, &result);
    source_arena_destroy(&scratch);
    if (!success) {
        return static_constraint_failure(solver, STATIC_CONSTRAINT_OUT_OF_MEMORY, site,
            "Static signature comparison could not allocate scratch", NULL, NULL);
    }
    StaticConstraintSignature *item = static_constraint_allocate(solver, sizeof(*item));
    if (item == NULL) { return false; }
    *item = (StaticConstraintSignature){.left = left, .right = right, .result = result, .next = solver->first_signature};
    solver->first_signature = item;
    solver->signature_count++;
    *output = result;
    return true;
}
