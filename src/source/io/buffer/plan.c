/**
 * @file
 * @brief Bound binary source growth and saturate the final geometric allocation without overflow.
 *
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) keeps EOF a byte
 * position, not appended source data. The final immutable copy needs n+1 bytes;
 * exact planning rejects n==SIZE_MAX before any buffer access or allocation.
 */
#include <stdint.h>
#include "buffer.h"

/** Prove addition fits first; capacity doubles until its one final saturation.
 * With no exact-growth tail, total copied prefixes remain a geometric sum plus
 * at most one final prefix. Output is committed only after the complete plan fits.
 */
SourceStatus source_buffer_plan(size_t length, size_t capacity, size_t addition, SourceBufferPlan *output) {
    if (output == NULL || length > capacity || capacity == SIZE_MAX) { return SOURCE_INVALID_ARGUMENT; }
    if (addition > (SIZE_MAX - 1) - length) { return SOURCE_LIMIT; }
    size_t needed = length + addition;
    if (needed > capacity) {
        capacity = needed > (SIZE_MAX - 1) / 2 ? SIZE_MAX - 1 : needed * 2;
    }
    *output = (SourceBufferPlan){needed, capacity};
    return SOURCE_OK;
}
