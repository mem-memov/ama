/** @file
 * @brief Query finite reachable metadata proofs without confusing validation cycles and value-definition cycles.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) requires all producer inputs and public definitions.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) releases query scratch unconditionally.
 */
#include "internal.h"

/** Every reachable actual proof is visited once. Unknown values remain Pending;
 * an active group is never treated as checked merely to close a cycle.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12).
 */
bool static_publication_dependencies_closed(StaticPublicationPlan *plan,
    StaticPublicationItem root, bool *output) {
    if (!static_publication_dependencies(plan)) { return false; }
    if (!output) {
        return static_publication_fail(plan, STATIC_PUBLICATION_INVALID, (SourceSpan){0},
            "Missing static closure outcome receiver");
    }
    StaticPublicationWork work = {.plan = plan, .closed = true};
    source_arena_init(&work.arena, plan->arena.allocator);
    bool success = static_publication_enqueue(&work, root);
    for (const StaticPublicationWorkNode *node = work.first; success && node; node = node->next) {
        success = static_publication_visit(&work, node->item);
    }
    source_arena_destroy(&work.arena);
    if (success) { *output = work.closed; }
    return success;
}
