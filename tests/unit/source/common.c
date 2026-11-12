/**
 * @file
 * @brief Own counted fail-at-N allocation and stable source-manager test storage.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../docs/implementation/library-tests-and-build.md)
 */
#ifndef AMA_SOURCE_TEST_COMMON
#define AMA_SOURCE_TEST_COMMON

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "source/text/source.h"

static SourceManager sources;
static size_t allocation_count;
static size_t failure_at;
static size_t live_allocations;

/**
 * Fail exactly the selected allocation ordinal, otherwise allocate and count live fixture
 * blocks; zero failure ordinal disables injection.
 */
static void *test_allocate(void *context, size_t size) {
    (void)context;
    allocation_count++;
    if (failure_at != 0 && allocation_count == failure_at) {
        return NULL;
    }
    void *allocation = malloc(size);
    if (allocation != NULL) {
        live_allocations++;
    }
    return allocation;
}

/**
 * Release one counted nonnull fixture allocation and assert the live counter cannot underflow;
 * null is a no-op.
 */
static void test_release(void *context, void *allocation) {
    (void)context;
    if (allocation != NULL) {
        assert(live_allocations != 0);
        live_allocations--;
        free(allocation);
    }
}

/**
 * Initialize this leaf fixture before its assertions; the enclosing file contract identifies
 * owned streams, source storage or model state. Setup failure is an assertion, never an accepted
 * product failure.
 */
static void setup_test(void) {
    allocation_count = 0;
    failure_at = 0;
    live_allocations = 0;
    SourceAllocator allocator = {NULL, test_allocate, test_release};
    source_manager_init(&sources, allocator);
}

/**
 * Release or reset this leaf fixture after observations, in reverse borrow order. Counted owners
 * assert zero remaining live allocations; assertions remain enabled by the leaf harness.
 */
static void destroy_test(void) {
    source_manager_destroy(&sources);
    assert(live_allocations == 0);
}

#endif
