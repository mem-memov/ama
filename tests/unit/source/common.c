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

static void test_release(void *context, void *allocation) {
    (void)context;
    if (allocation != NULL) {
        assert(live_allocations != 0);
        live_allocations--;
        free(allocation);
    }
}

static void setup_test(void) {
    allocation_count = 0;
    failure_at = 0;
    live_allocations = 0;
    SourceAllocator allocator = {NULL, test_allocate, test_release};
    source_manager_init(&sources, allocator);
}

static void destroy_test(void) {
    source_manager_destroy(&sources);
    assert(live_allocations == 0);
}

#endif
