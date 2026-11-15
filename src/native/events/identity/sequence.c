/** @file
 * @brief Reserve strictly increasing native identities before publication.
 * [ASYNC-05](../../../../spec/language/async-and-parallelism.md#async-05)
 * rejects exhausted service/submission counters instead of wrapping into old tokens.
 */
#include "native/events/internal.h"
bool ama_native_event_sequence(uint64_t current, uint64_t limit, uint64_t *next) {
    if (current >= limit) { return false; }
    *next = current + 1;
    return true;
}
