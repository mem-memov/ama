/** @file
 * @brief Bound query-local work lifetime while preserving sticky solver allocation errors.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) separates persistent facts and temporary work.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) avoids reporting allocations.
 */
#include "internal.h"

/** The caller owns scratch initialization/teardown; no pointer to this storage
 * may escape into persistent terms, facts or equations.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) bounds partial work.
 */
void *static_constraint_scratch(StaticConstraintSolver *solver, SourceArena *scratch, size_t size) {
    if (solver->status != STATIC_CONSTRAINT_OK) { return NULL; }
    void *record = source_arena_allocate(scratch, size);
    if (record == NULL) {
        (void)static_constraint_failure(solver, STATIC_CONSTRAINT_OUT_OF_MEMORY, (SourceSpan){0},
            "Static constraint query could not allocate scratch", NULL, NULL);
    }
    return record;
}
