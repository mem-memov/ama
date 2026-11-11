#include "lifetime.h"

AmaStatus ama_lifetime_begin(AmaLifetime *lifetime, size_t children, AmaConstructorKind kind) {
    if (lifetime->phase != AMA_LIFETIME_FRESH || lifetime->native_live ||
        lifetime->live_children != 0 || lifetime->cleanup_active ||
        kind < AMA_CONSTRUCT_DEFAULT || kind > AMA_CONSTRUCT_CLONE) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    lifetime->phase = AMA_LIFETIME_CONSTRUCTING;
    lifetime->constructor = kind;
    lifetime->child_capacity = children;
    return ama_status_success();
}

AmaStatus ama_lifetime_child_ready(AmaLifetime *lifetime) {
    if (lifetime->phase != AMA_LIFETIME_CONSTRUCTING ||
        lifetime->live_children == lifetime->child_capacity) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    lifetime->live_children++;
    return ama_status_success();
}

AmaStatus ama_lifetime_finish(AmaLifetime *lifetime, bool native_constructed) {
    if (lifetime->phase != AMA_LIFETIME_CONSTRUCTING ||
        lifetime->live_children != lifetime->child_capacity) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    lifetime->native_live = native_constructed;
    lifetime->phase = AMA_LIFETIME_LIVE;
    return ama_status_success();
}
