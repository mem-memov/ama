/** @file
 * @brief Own candidate/use allocations without owning borrowed names, guards or source values.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) preserves unconditional partial teardown.
 */
#include <assert.h>
#include <string.h>
#include "internal.h"

/** Initialization is allocation-free; no key or source body is prepared.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
void static_registry_init(StaticRegistryPlan *plan, StaticGuardPlan *guards,
    IdentityRegistry *names, SourceAllocator allocator) {
    assert(plan && guards && names);
    *plan = (StaticRegistryPlan){.guards = guards, .names = names};
    source_arena_init(&plan->arena, allocator);
}

/** Destruction follows no borrowed graph edges and releases partial records.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
void static_registry_destroy(StaticRegistryPlan *plan) {
    source_arena_destroy(&plan->arena);
    *plan = (StaticRegistryPlan){0};
}

/** All preparation remains arena-owned after any later child failure.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
void *static_registry_allocate(StaticRegistryPlan *plan, size_t length) {
    if (plan->status != STATIC_REGISTRY_OK) { return NULL; }
    void *record = source_arena_allocate(&plan->arena, length);
    if (record == NULL) {
        (void)static_registry_fail(plan, STATIC_REGISTRY_OUT_OF_MEMORY, (SourceSpan){0},
            "Specialization registry could not allocate metadata");
    } else { memset(record, 0, length); }
    return record;
}
