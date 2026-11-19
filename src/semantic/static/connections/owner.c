/** @file
 * @brief Own lexical binding metadata separately from borrowed static semantic stages.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns complete/failed preparation.
 */
#include <assert.h>
#include <string.h>
#include "internal.h"

/** Initialization neither allocates nor evaluates source/static work.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) separates ownership.
 */
void static_connection_init(StaticConnectionPlan *plan, StaticConstantPlan *constants, SourceAllocator allocator) {
    assert(constants && constants->status == STATIC_CONSTANT_OK);
    *plan = (StaticConnectionPlan){.constants = constants};
    source_arena_init(&plan->arena, allocator);
}

/** Destruction releases only the finite arena, not borrowed AST/Type records.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) bounds teardown.
 */
void static_connection_destroy(StaticConnectionPlan *plan) {
    source_arena_destroy(&plan->arena);
    *plan = (StaticConnectionPlan){0};
}

/** Failed allocation retains all previously prepared descriptors for teardown.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns refusal state.
 */
void *static_connection_allocate(StaticConnectionPlan *plan, size_t size) {
    if (plan->status != STATIC_CONNECTION_OK) { return NULL; }
    void *value = source_arena_allocate(&plan->arena, size);
    if (value == NULL) {
        (void)static_connection_fail(plan, STATIC_CONNECTION_OUT_OF_MEMORY, (SourceSpan){0}, "Typed connection metadata could not allocate storage");
    } else { memset(value, 0, size); }
    return value;
}
