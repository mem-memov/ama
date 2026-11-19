/** @file
 * @brief Own partial required-context worklists without owning their static source/equation dependencies.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) separates borrowed and owned lifetimes.
 */
#include <assert.h>
#include <string.h>
#include "internal.h"

/** Initialization creates no context, root, equation or allocation.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) requires this explicit partial-owner lifetime and teardown.
 */
void static_closure_init(StaticClosurePlan *plan, StaticConnectionPlan *connections, SourceAllocator allocator) {
    assert(connections && connections->status == STATIC_CONNECTION_OK);
    *plan = (StaticClosurePlan){.connections = connections};
    source_arena_init(&plan->arena, allocator);
}

/** Release arena records without traversing borrowed cyclic Type dependencies.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) requires this explicit partial-owner lifetime and teardown.
 */
void static_closure_destroy(StaticClosurePlan *plan) {
    source_arena_destroy(&plan->arena);
    *plan = (StaticClosurePlan){0};
}

/** Failed preparation stays owned; reporting does not allocate another record.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) requires this explicit partial-owner lifetime and teardown.
 */
void *static_closure_allocate(StaticClosurePlan *plan, size_t size) {
    if (plan->status != STATIC_CLOSURE_OK) { return NULL; }
    void *record = source_arena_allocate(&plan->arena, size);
    if (!record) {
        (void)static_closure_fail(plan, STATIC_CLOSURE_OUT_OF_MEMORY, (SourceSpan){0}, "Required specialization worklist could not allocate storage");
    } else { memset(record, 0, size); }
    return record;
}
