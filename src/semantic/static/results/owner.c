/** @file
 * @brief Own static result preparation separately from borrowed registry, guard and term lifetimes.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) retains failed construction.
 */
#include <assert.h>
#include <string.h>
#include "internal.h"

/** Initializing this owner cannot execute or resolve any static expression.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) separates ownership.
 */
void static_result_init(StaticResultPlan *plan, StaticRegistryPlan *registry, SourceAllocator allocator) {
    assert(registry && registry->status == STATIC_REGISTRY_OK && registry->guards->status == STATIC_GUARD_OK);
    *plan = (StaticResultPlan){.registry = registry, .guards = registry->guards};
    source_arena_init(&plan->arena, allocator);
}

/** Teardown follows owned allocations only, not source/type/definition edges.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) bounds cleanup.
 */
void static_result_destroy(StaticResultPlan *plan) {
    source_arena_destroy(&plan->arena);
    *plan = (StaticResultPlan){0};
}

/** Every unpublished record remains owned even when a borrowed operation fails.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns partial preparation.
 */
void *static_result_allocate(StaticResultPlan *plan, size_t size) {
    if (plan->status != STATIC_RESULT_OK) { return NULL; }
    void *value = source_arena_allocate(&plan->arena, size);
    if (value == NULL) {
        (void)static_result_fail(plan, STATIC_RESULT_OUT_OF_MEMORY, (SourceSpan){0},
            "Static result preparation could not allocate storage");
    } else { memset(value, 0, size); }
    return value;
}
