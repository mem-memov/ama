/** @file
 * @brief Exhaust closed write-join ownership using each leaf's genuine counted source fixture.
 * @see [T276 resumed review](../../../../../../docs/implementation/source-operations.md#t276-resumed-entry-review-after-t285)
 */
#ifndef AMA_SOURCE_WRITE_JOIN_TEST_COMMON
#define AMA_SOURCE_WRITE_JOIN_TEST_COMMON
#include "../../../../../../tools/project/source/operation/writes/join/join.h"
/** Run after the enclosing counted source fixture and producer owners exist.
 * Every allocation refusal retains a destructible prefix and changes no borrowed
 * map, applied text or valid input owner. Allocation-free joins remain valid. */
static void check_source_write_join_failures(const SourceOperationWritesInputs *inputs) {
    size_t total = 0, baseline = live_allocations;
    for (size_t failure = 0; !failure || failure <= total; failure++) {
        SourceOperationWrites writes; source_operation_writes_init(&writes, sources.allocator);
        size_t before = allocation_count; if (failure) { failure_at = before + failure; }
        SourceOperationWritesStatus status = source_operation_writes_build(&writes, inputs);
        if (!failure) {
            assert(status == SOURCE_OPERATION_WRITES_OK && writes.valid); total = allocation_count - before;
            size_t repeated = allocation_count;
            assert(source_operation_writes_build(&writes, inputs) == SOURCE_OPERATION_WRITES_INVALID && writes.valid && allocation_count == repeated);
        } else { assert(status == SOURCE_OPERATION_WRITES_MEMORY && !writes.valid); }
        failure_at = 0; source_operation_writes_destroy(&writes); assert(live_allocations == baseline);
        assert(inputs->mains->ready && inputs->files->ready);
    }
}
#endif
