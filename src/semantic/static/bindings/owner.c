/**
 * @file
 * @brief Own compile-time binding records without changing borrowed static inventories.
 *
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) gives partial
 * construction one deterministic teardown boundary. No field, target or runtime
 * instance is acquired while metadata is prepared.
 */
#include <string.h>
#include "internal.h"

/** No allocation or mutation of another semantic owner occurs at initialization.
 * Contract: [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
void binding_plan_init(BindingPlan *plan, SourceAllocator allocator) {
    *plan = (BindingPlan){0};
    source_arena_init(&plan->arena, allocator);
}

/** Arena destruction ignores snapshot/borrowed graph edges, including partially
 * built unpublished records; callers keep borrowed sources alive until here.
 * Contract: [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
void binding_plan_destroy(BindingPlan *plan) {
    source_arena_destroy(&plan->arena);
    *plan = (BindingPlan){0};
}

/** Allocate only while healthy; zero complete successful records before clients
 * assign semantic identities or link them into any snapshot.
 * Contract: [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
void *binding_allocate(BindingPlan *plan, size_t size) {
    if (plan->status != BINDING_OK) { return NULL; }
    void *result = source_arena_allocate(&plan->arena, size);
    if (result == NULL) { (void)binding_memory_failure(plan); }
    else { memset(result, 0, size); }
    return result;
}
