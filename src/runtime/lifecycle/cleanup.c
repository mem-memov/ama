#include "lifetime.h"

AmaStatus ama_lifetime_begin_cleanup(AmaLifetime *lifetime) {
    if (lifetime->cleanup_active || (lifetime->phase != AMA_LIFETIME_LIVE &&
        lifetime->phase != AMA_LIFETIME_CONSTRUCTING)) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    lifetime->phase = AMA_LIFETIME_FINALIZED;
    lifetime->cleanup_active = true;
    return ama_status_success();
}

bool ama_lifetime_take_native(AmaLifetime *lifetime) {
    if (!lifetime->cleanup_active || !lifetime->native_live) {
        return false;
    }
    lifetime->native_live = false;
    return true;
}

bool ama_lifetime_next_child(AmaLifetime *lifetime, size_t *index) {
    if (!lifetime->cleanup_active || lifetime->native_live || lifetime->live_children == 0) {
        return false;
    }
    *index = --lifetime->live_children;
    return true;
}

AmaStatus ama_lifetime_end_cleanup(AmaLifetime *lifetime) {
    if (!lifetime->cleanup_active || lifetime->native_live || lifetime->live_children != 0) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    lifetime->cleanup_active = false;
    return ama_status_success();
}
