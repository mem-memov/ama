/**
 * @file
 * @brief Consume the constructed resource prefix exactly once during cleanup.
 *
 * [LIFE-03](../../../spec/implementation/native-interface.md#life-03)
 * Clear native ownership before calling a destructor, then enumerate children in reverse construction order.
 */
#include "lifetime.h"

/** Enter cleanup from Live or Constructing exactly once and mark Finalized before any user destructor can run. Component contract: [LIFE-03](../../../spec/implementation/native-interface.md#life-03). */
AmaStatus ama_lifetime_begin_cleanup(AmaLifetime *lifetime) {
    if (lifetime->cleanup_active || (lifetime->phase != AMA_LIFETIME_LIVE &&
        lifetime->phase != AMA_LIFETIME_CONSTRUCTING)) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    lifetime->phase = AMA_LIFETIME_FINALIZED;
    lifetime->cleanup_active = true;
    return ama_status_success();
}

/** Consume the native-live bit before returning true, preventing duplicate destructor invocation even when that destructor fails. Component contract: [LIFE-03](../../../spec/implementation/native-interface.md#life-03). */
bool ama_lifetime_take_native(AmaLifetime *lifetime) {
    if (!lifetime->cleanup_active || !lifetime->native_live) {
        return false;
    }
    lifetime->native_live = false;
    return true;
}

/** After native cleanup, consume the next reverse child index from the initialized prefix; false leaves output unchanged. Component contract: [LIFE-03](../../../spec/implementation/native-interface.md#life-03). */
bool ama_lifetime_next_child(AmaLifetime *lifetime, size_t *index) {
    if (!lifetime->cleanup_active || lifetime->native_live || lifetime->live_children == 0) {
        return false;
    }
    *index = --lifetime->live_children;
    return true;
}

/** Finish only after native and child credits are exhausted; no allocation or payload access. Component contract: [LIFE-03](../../../spec/implementation/native-interface.md#life-03). */
AmaStatus ama_lifetime_end_cleanup(AmaLifetime *lifetime) {
    if (!lifetime->cleanup_active || lifetime->native_live || lifetime->live_children != 0) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    lifetime->cleanup_active = false;
    return ama_status_success();
}
