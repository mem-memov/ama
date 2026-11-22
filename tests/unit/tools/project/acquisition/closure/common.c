/** @file
 * @brief Own only immutable bytes and result allocation needed by closure component tests.
 * Parsing/routing fixture operations stay in their own acquisition test parent.
 * @see [T235 contract](../../../../../../docs/implementation/acquisition-plan.md#t235-structural-source-and-test-delivery)
 */
#ifndef AMA_ACQUISITION_CLOSURE_TEST_COMMON
#define AMA_ACQUISITION_CLOSURE_TEST_COMMON
#include <stdio.h>
#include "../../../../source/configuration/common.c"
#include "source/memory/arena.h"
static SourceArena results;
/** Begin counted immutable byte storage and independently owned result metadata. */
static void begin(void) { setup_test(); source_arena_init(&results, sources.allocator); }
/** Release result paths before their borrowed byte sources and check all allocations. */
static void finish(void) { source_arena_destroy(&results); destroy_test(); }
#endif
