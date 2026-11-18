/** @file
 * @brief Compose guard ownership with actual application, solver, term and source fixtures.
 */
#ifndef AMA_STATIC_GUARD_TEST_COMMON
#define AMA_STATIC_GUARD_TEST_COMMON
#include "../applications/common.c"
#include "semantic/static/guards/internal.h"

/** Guard metadata expires before every application/source object it borrows. */
static StaticGuardPlan guards;

/** Build healthy dependencies before the no-allocation guard initialization. */
static void setup_guards(const char *caller, const char *target) {
    setup_applications(caller, target);
    size_t before = allocation_count;
    static_guard_init(&guards, &applications, sources.allocator);
    assert(guards.status == STATIC_GUARD_OK && allocation_count == before);
}

/** Release the guard first; underlying counted teardown asserts no leaked block. */
static void destroy_guards(void) {
    static_guard_destroy(&guards);
    assert(guards.applications == NULL && guards.first_owner == NULL && guards.owner_count == 0);
    destroy_applications();
}
#endif
