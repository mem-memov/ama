/** @file
 * @brief Own publication descriptors independently from borrowed static evaluation state.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) bounds partial ownership.
 */
#include <assert.h>
#include <string.h>
#include "internal.h"

/** Initialization publishes no static fact and performs no allocation.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) separates owners.
 */
void static_publication_init(StaticPublicationPlan *plan, StaticResultPlan *results, SourceAllocator allocator) {
    assert(results && results->status == STATIC_RESULT_OK);
    *plan = (StaticPublicationPlan){.results = results};
    source_arena_init(&plan->arena, allocator);
}

/** Only the arena is traversed; borrowed semantic graphs remain untouched.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) bounds destruction.
 */
void static_publication_destroy(StaticPublicationPlan *plan) {
    source_arena_destroy(&plan->arena);
    *plan = (StaticPublicationPlan){0};
}

/** Failed descriptors remain owned until deterministic teardown.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns partial preparation.
 */
void *static_publication_allocate(StaticPublicationPlan *plan, size_t size) {
    if (plan->status != STATIC_PUBLICATION_OK) { return NULL; }
    void *value = source_arena_allocate(&plan->arena, size);
    if (value == NULL) {
        (void)static_publication_fail(plan, STATIC_PUBLICATION_OUT_OF_MEMORY, (SourceSpan){0},
            "Static publication could not allocate storage");
    } else { memset(value, 0, size); }
    return value;
}
