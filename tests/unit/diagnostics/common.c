/**
 * @file
 * @brief Own stable Unicode source storage used by diagnostic record and rendering tests.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../docs/implementation/library-tests-and-build.md)
 */
#ifndef AMA_DIAGNOSTIC_TEST_COMMON
#define AMA_DIAGNOSTIC_TEST_COMMON

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "source/text/source.h"
#include "diagnostics/record/diagnostic.h"

static SourceManager sources;
static const SourceText *source;

/**
 * Initialize this leaf fixture before its assertions; the enclosing file contract identifies
 * owned streams, source storage or model state. Setup failure is an assertion, never an accepted
 * product failure.
 */
static void setup_test(void) {
    source_manager_init(&sources, source_system_allocator());
    const unsigned char bytes[] = "@!\n商品\n.shop.item\n";
    SourceStatus status = source_manager_add(&sources, "商店/item", bytes, sizeof(bytes) - 1, &source);
    assert(status == SOURCE_OK);
}

/**
 * Release or reset this leaf fixture after observations, in reverse borrow order. Counted owners
 * assert zero remaining live allocations; assertions remain enabled by the leaf harness.
 */
static void destroy_test(void) {
    source_manager_destroy(&sources);
}

#endif
