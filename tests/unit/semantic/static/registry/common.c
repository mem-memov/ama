/** @file
 * @brief Own counted registry fixtures while retaining longer-lived source, guard and name owners.
 */
#ifndef AMA_STATIC_REGISTRY_TEST_COMMON
#define AMA_STATIC_REGISTRY_TEST_COMMON
#include "identity/common.c"
#include "semantic/static/registry/internal.h"

/** Names outlive registry records; all metadata expires before source owners. */
static IdentityRegistry registry_names;
static StaticRegistryPlan registry;

/** No names or candidates are allocated during registry initialization. */
static void setup_registry(const char *caller, const char *target) {
    setup_guards(caller, target);
    identity_registry_init(&registry_names, sources.allocator);
    size_t before = allocation_count;
    static_registry_init(&registry, &guards, &registry_names, sources.allocator);
    assert(registry.status == STATIC_REGISTRY_OK && allocation_count == before);
}

/** Both owned arenas release partial preparation before borrowed guard teardown. */
static void destroy_registry(void) {
    static_registry_destroy(&registry);
    assert(registry.guards == NULL && registry.first_use == NULL && registry.first_candidate == NULL);
    identity_registry_destroy(&registry_names);
    destroy_guards();
}

#endif
