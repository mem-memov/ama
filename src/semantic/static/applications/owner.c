/** @file
 * @brief Own structural application preparation independently of borrowed source and solution graphs.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns partial construction.
 */
#include <assert.h>
#include <string.h>
#include "internal.h"

/** Empty initialization performs no lookup, application or allocation.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) separates lifetimes.
 */
void static_application_plan_init(StaticApplicationPlan *plan, StaticTermStore *terms,
    StaticConstraintSolver *solver, BindingPlan *bindings, SourceAllocator allocator) {
    assert(terms && terms->status == STATIC_TERM_OK && solver && solver->terms == terms &&
        solver->status == STATIC_CONSTRAINT_OK && bindings && bindings->status == BINDING_OK);
    *plan = (StaticApplicationPlan){.terms = terms, .solver = solver, .bindings = bindings};
    source_arena_init(&plan->arena, allocator);
}

/** Cleanup follows owned allocations, never source or symbolic dependency edges.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) bounds cleanup.
 */
void static_application_plan_destroy(StaticApplicationPlan *plan) {
    source_arena_destroy(&plan->arena);
    *plan = (StaticApplicationPlan){0};
}

/** Refusal keeps every partial object destructible and never requires another
 * allocation to explain it. [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
void *static_application_allocate(StaticApplicationPlan *plan, size_t size) {
    if (plan->status != STATIC_APPLY_OK) { return NULL; }
    void *record = source_arena_allocate(&plan->arena, size);
    if (record == NULL) {
        (void)static_application_fail(plan, STATIC_APPLY_OUT_OF_MEMORY, (SourceSpan){0},
            "Static application could not allocate storage", NULL);
    } else { memset(record, 0, size); }
    return record;
}
