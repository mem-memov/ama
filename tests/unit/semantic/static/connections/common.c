/** @file
 * @brief Own typed connection fixtures before every borrowed static/source dependency.
 */
#ifndef AMA_STATIC_CONNECTION_TEST_COMMON
#define AMA_STATIC_CONNECTION_TEST_COMMON
#include "../constants/common.c"
#include "semantic/static/connections/internal.h"

/** Partial source bindings expire before all borrowed semantic owners. */
static StaticConnectionPlan connections;

/** Initialization allocates nothing and executes no static/runtime source work. */
static void setup_connections(const char *caller, const char *target) {
    setup_constants(caller, target);
    size_t before = allocation_count;
    static_connection_init(&connections, &constants, sources.allocator);
    assert(connections.status == STATIC_CONNECTION_OK && allocation_count == before);
}

/** Teardown follows the exact borrow order and proves zero counted allocations. */
static void destroy_connections(void) {
    static_connection_destroy(&connections);
    assert(connections.constants == NULL && connections.first_runtime == NULL);
    destroy_constants();
}
#endif
