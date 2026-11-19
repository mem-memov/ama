/** @file
 * @brief Own static semantic children in dependency order without owning the original source inventory.
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03) makes failed-stage teardown unconditional.
 */
#include <assert.h>
#include "analysis.h"

/** Each initializer borrows only already initialized predecessors and allocates
 * nothing, so initialization cannot leave a fallible partial credit sequence.
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03) separates resource acquisition from owner setup.
 */
void compiler_specialization_init(CompilerSpecialization *analysis, const StaticInventory *inventory, SourceAllocator allocator) {
    assert(inventory && inventory->valid);
    *analysis = (CompilerSpecialization){.inventory = inventory};
    binding_plan_init(&analysis->bindings, allocator);
    identity_registry_init(&analysis->identities, allocator);
    static_term_store_init(&analysis->terms, inventory, allocator);
    static_constraint_init(&analysis->constraints, &analysis->terms, allocator);
    static_application_plan_init(&analysis->applications, &analysis->terms, &analysis->constraints, &analysis->bindings, allocator);
    static_guard_init(&analysis->guards, &analysis->applications, allocator);
    static_registry_init(&analysis->registry, &analysis->guards, &analysis->identities, allocator);
    static_result_init(&analysis->results, &analysis->registry, allocator);
    static_publication_init(&analysis->publications, &analysis->results, allocator);
    static_constant_init(&analysis->constants, &analysis->publications, allocator);
    static_connection_init(&analysis->connections, &analysis->constants, allocator);
    static_closure_init(&analysis->closure, &analysis->connections, allocator);
}

/** Release borrowers before their dependencies; arena destructors do not follow
 * recursive nominal references or execute source/native bodies.
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03) preserves diagnostic/source lifetime until outer teardown.
 */
void compiler_specialization_destroy(CompilerSpecialization *analysis) {
    static_closure_destroy(&analysis->closure);
    static_connection_destroy(&analysis->connections);
    static_constant_destroy(&analysis->constants);
    static_publication_destroy(&analysis->publications);
    static_result_destroy(&analysis->results);
    static_registry_destroy(&analysis->registry);
    static_guard_destroy(&analysis->guards);
    static_application_plan_destroy(&analysis->applications);
    static_constraint_destroy(&analysis->constraints);
    static_term_store_destroy(&analysis->terms);
    identity_registry_destroy(&analysis->identities);
    binding_plan_destroy(&analysis->bindings);
    *analysis = (CompilerSpecialization){0};
}
