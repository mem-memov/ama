/**
 * @file
 * @brief Track the one selected constructor and successfully completed child prefix.
 *
 * [LIFE-02](../../../spec/implementation/native-interface.md#life-02)
 * Failed native construction never receives a destructor credit; publication requires every expected child.
 */
#include "lifetime.h"

/** Require fresh, unowned metadata and a valid selected constructor; record expected child capacity without invoking a hook. Invalid state leaves the ledger unchanged. */
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

/** Credit one successfully initialized child in declaration order. Require constructing state and unused prefix capacity; never credit a failed child. */
AmaStatus ama_lifetime_child_ready(AmaLifetime *lifetime) {
    if (lifetime->phase != AMA_LIFETIME_CONSTRUCTING ||
        lifetime->live_children == lifetime->child_capacity) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    lifetime->live_children++;
    return ama_status_success();
}

/** Publish lifecycle Live only after the entire child prefix is complete. The caller supplies whether one native resource was successfully constructed. */
AmaStatus ama_lifetime_finish(AmaLifetime *lifetime, bool native_constructed) {
    if (lifetime->phase != AMA_LIFETIME_CONSTRUCTING ||
        lifetime->live_children != lifetime->child_capacity) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    lifetime->native_live = native_constructed;
    lifetime->phase = AMA_LIFETIME_LIVE;
    return ama_status_success();
}
