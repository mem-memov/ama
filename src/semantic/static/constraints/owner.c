/** @file
 * @brief Own incremental constraint storage without changing borrowed term payloads.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns partial construction.
 */
#include <assert.h>
#include <string.h>
#include "internal.h"

/** Initialization has no allocation, source lookup or implicit solve.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) separates owners.
 */
void static_constraint_init(StaticConstraintSolver *solver, StaticTermStore *terms, SourceAllocator allocator) {
    assert(terms != NULL && terms->status == STATIC_TERM_OK);
    *solver = (StaticConstraintSolver){.terms = terms, .dirty = true};
    source_arena_init(&solver->arena, allocator);
}

/** Arena teardown follows allocation ownership, never equation/capture cycles.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) bounds cleanup.
 */
void static_constraint_destroy(StaticConstraintSolver *solver) {
    source_arena_destroy(&solver->arena);
    *solver = (StaticConstraintSolver){0};
}

/** Partial failures keep prior publications and require no reporting allocation.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns partial storage.
 */
void *static_constraint_allocate(StaticConstraintSolver *solver, size_t size) {
    if (solver->status != STATIC_CONSTRAINT_OK) { return NULL; }
    void *record = source_arena_allocate(&solver->arena, size);
    if (record == NULL) {
        (void)static_constraint_failure(solver, STATIC_CONSTRAINT_OUT_OF_MEMORY, (SourceSpan){0},
            "Static constraints could not allocate storage", NULL, NULL);
    } else { memset(record, 0, size); }
    return record;
}
