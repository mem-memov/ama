/**
 * @file
 * @brief Check complete numeric read-buffer bounds, final saturation and alias-safe transactional appends.
 */
#include <stdint.h>
#include "source/io/buffer/buffer.h"

/** Exact endpoint checks cover representable source/terminator arithmetic without enormous allocations. */
static void it_bounds_source_buffer_growth_and_saturates_without_an_exact_growth_tail(void) {
    SourceBufferPlan plan = {11,22};
    assert(source_buffer_plan(0, 0, 0, &plan) == SOURCE_OK && plan.length == 0 && plan.capacity == 0);
    assert(source_buffer_plan(3, 8, 5, &plan) == SOURCE_OK && plan.length == 8 && plan.capacity == 8);
    assert(source_buffer_plan(8, 8, 1, &plan) == SOURCE_OK && plan.length == 9 && plan.capacity >= 18);
    assert(source_buffer_plan(SIZE_MAX / 2, SIZE_MAX / 2, 1, &plan) == SOURCE_OK);
    assert(plan.length == SIZE_MAX / 2 + 1 && plan.capacity == SIZE_MAX - 1);
    assert(source_buffer_plan(SIZE_MAX - 2, SIZE_MAX - 1, 1, &plan) == SOURCE_OK && plan.length == SIZE_MAX - 1);
    SourceBufferPlan unchanged = plan;
    assert(source_buffer_plan(SIZE_MAX - 1, SIZE_MAX - 1, 1, &plan) == SOURCE_LIMIT);
    assert(plan.length == unchanged.length && plan.capacity == unchanged.capacity);
    assert(source_buffer_plan(0, 0, SIZE_MAX, &plan) == SOURCE_LIMIT);
    assert(source_buffer_plan(3, 2, 0, &plan) == SOURCE_INVALID_ARGUMENT);
    assert(source_buffer_plan(0, SIZE_MAX, 0, &plan) == SOURCE_INVALID_ARGUMENT);
    assert(source_buffer_plan(0, 0, 0, NULL) == SOURCE_INVALID_ARGUMENT);
    assert(plan.length == unchanged.length && plan.capacity == unchanged.capacity);
    for (size_t length = 0; length <= 32; length++) {
        for (size_t capacity = length; capacity <= 64; capacity++) {
            for (size_t addition = 0; addition <= 96; addition++) {
                assert(source_buffer_plan(length, capacity, addition, &plan) == SOURCE_OK);
                assert(plan.length == length + addition && plan.capacity >= plan.length && plan.capacity >= capacity);
                if (plan.length <= capacity) { assert(plan.capacity == capacity); }
                else { assert(plan.capacity >= 2 * plan.length); }
            }
        }
    }
}

/** Appending an internal prefix works before/after growth; allocation failure preserves all old bytes and ownership. */
static void it_appends_aliasing_chunks_without_use_after_free_or_partial_commit(void) {
    setup_test();
    SourceBuffer buffer = {0};
    assert(source_buffer_append(sources.allocator, &buffer, (const unsigned char *)"abc", 3) == SOURCE_OK);
    assert(buffer.length == 3 && buffer.capacity == 6);
    assert(source_buffer_append(sources.allocator, &buffer, buffer.bytes, 3) == SOURCE_OK);
    assert(buffer.length == 6 && memcmp(buffer.bytes, "abcabc", 6) == 0);
    unsigned char *old = buffer.bytes;
    size_t capacity = buffer.capacity;
    failure_at = allocation_count + 1;
    assert(source_buffer_append(sources.allocator, &buffer, buffer.bytes, 6) == SOURCE_OUT_OF_MEMORY);
    assert(buffer.bytes == old && buffer.capacity == capacity && buffer.length == 6);
    assert(memcmp(buffer.bytes, "abcabc", 6) == 0 && live_allocations == 1);
    failure_at = 0;
    assert(source_buffer_append(sources.allocator, &buffer, buffer.bytes, 6) == SOURCE_OK);
    assert(buffer.length == 12 && memcmp(buffer.bytes, "abcabcabcabc", 12) == 0);
    old = buffer.bytes;
    size_t before = allocation_count;
    assert(source_buffer_append(sources.allocator, &buffer, NULL, 0) == SOURCE_OK);
    assert(buffer.bytes == old && allocation_count == before && buffer.length == 12);
    assert(source_buffer_append(sources.allocator, &buffer, NULL, 1) == SOURCE_INVALID_ARGUMENT);
    test_release(NULL, buffer.bytes);
    destroy_test();
}
