/** @file
 * @brief Own finite guard records without owning or executing their borrowed applications.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) separates failed construction.
 */
#include <assert.h>
#include <string.h>
#include "internal.h"

/** Initialization cannot resolve a Type, expand a definition or allocate storage.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) separates ownership.
 */
void static_guard_init(StaticGuardPlan *plan, StaticApplicationPlan *applications, SourceAllocator allocator) {
    assert(applications && applications->status == STATIC_APPLY_OK);
    *plan = (StaticGuardPlan){.applications = applications};
    source_arena_init(&plan->arena, allocator);
    source_arena_init(&plan->witness_arena, allocator);
}

/** Free partial records without following borrowed value/source dependencies.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) bounds cleanup.
 */
void static_guard_destroy(StaticGuardPlan *plan) {
    source_arena_destroy(&plan->arena);
    source_arena_destroy(&plan->witness_arena);
    *plan = (StaticGuardPlan){0};
}

/** Retain every unpublished allocation so failure teardown is unconditional.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns partial records.
 */
void *static_guard_allocate(StaticGuardPlan *plan, size_t size) {
    if (plan->status != STATIC_GUARD_OK) { return NULL; }
    void *value = source_arena_allocate(&plan->arena, size);
    if (value == NULL) {
        (void)static_guard_fail(plan, STATIC_GUARD_OUT_OF_MEMORY, (SourceSpan){0},
            "Static guard could not allocate storage");
    } else { memset(value, 0, size); }
    return value;
}
