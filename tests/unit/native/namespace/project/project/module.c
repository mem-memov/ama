/**
 * @file
 * @brief Define project-root companion hooks, helpers and immutable objects under the same prefix as its header.
 */
#include "module.h"
/** Provider-local reason; no unqualified file-scope author identifier is introduced. */
enum { AMA_NATIVE(reason) = 7 };
/** Immutable provider-local seed; identical local spelling is safe across companions. */
static const int AMA_NATIVE(seed) = 21;
/** Read immutable local data through a module-qualified private helper. */
static int AMA_NATIVE(read_seed)(void) { return AMA_NATIVE(seed); }
/** Initialize the caller's payload from this module's private constant. */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *state) {
    state->value = AMA_NATIVE(read_seed)();
    return ama_status_success();
}
/** Copy native value state into a distinct destination without shared ownership. */
AmaStatus AMA_NATIVE(clone)(AMA_NATIVE(State) *destination, const AMA_NATIVE(State) *source) {
    *destination = *source;
    return ama_status_success();
}
/** Preserve program-lifetime message bytes while assigning the generated provider namespace. */
AmaStatus AMA_NATIVE(failure)(void) {
    return AMA_NATIVE_ERROR(AMA_NATIVE(reason), "project", 7);
}
/** Check single evaluation with three disjoint counters; evaluation order is immaterial. */
AmaStatus AMA_NATIVE(evaluate_once)(size_t counts[3]) {
    return AMA_NATIVE_ERROR((counts[0]++, AMA_NATIVE(reason)),
        (counts[1]++, "project"), (counts[2]++, 7));
}
