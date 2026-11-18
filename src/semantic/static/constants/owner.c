/** @file
 * @brief Own constant type metadata separately from canonical static values and runtime shared storage.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) bounds failed construction.
 */
#include <assert.h>
#include <string.h>
#include "internal.h"

/** Initialization publishes no evidence and executes no constructors.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) separates ownership.
 */
void static_constant_init(StaticConstantPlan *plan, StaticPublicationPlan *publications, SourceAllocator allocator) {
    assert(publications && publications->status == STATIC_PUBLICATION_OK);
    *plan = (StaticConstantPlan){.publications = publications};
    source_arena_init(&plan->arena, allocator);
}

/** Arena teardown never follows borrowed context, source or term links.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) bounds destruction.
 */
void static_constant_destroy(StaticConstantPlan *plan) {
    source_arena_destroy(&plan->arena);
    *plan = (StaticConstantPlan){0};
}

/** Failed allocation leaves every partial descriptor owned until destruction.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns partial records.
 */
void *static_constant_allocate(StaticConstantPlan *plan, size_t size) {
    if (plan->status != STATIC_CONSTANT_OK) { return NULL; }
    void *value = source_arena_allocate(&plan->arena, size);
    if (value == NULL) {
        (void)static_constant_fail(plan, STATIC_CONSTANT_OUT_OF_MEMORY, (SourceSpan){0},
            "Static constant metadata could not allocate storage");
    } else { memset(value, 0, size); }
    return value;
}
