#include "identity.h"

bool ama_handle_is_empty(AmaHandle handle) {
    return handle.generation == 0;
}

AmaStatus ama_validate_handle(AmaHandle handle, uint64_t expected_type,
    const AmaSlotIdentity *slot) {
    if (ama_handle_is_empty(handle)) {
        return ama_status_make(AMA_EMPTY_REFERENCE, NULL, 0);
    }
    if (expected_type == 0) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    if (handle.type_id != expected_type) {
        return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0);
    }
    if (slot == NULL || !slot->live || slot->retired || handle.generation == 0 ||
        slot->type_id != handle.type_id || slot->slot_id != handle.slot_id ||
        slot->generation != handle.generation) {
        return ama_status_make(AMA_STALE_REFERENCE, NULL, 0);
    }
    return ama_status_success();
}

AmaStatus ama_slot_advance_generation_limited(AmaSlotIdentity *slot, uint64_t limit) {
    if (slot->live) {
        return ama_status_make(AMA_BUSY, NULL, 0);
    }
    if (slot->retired) {
        return ama_status_make(AMA_STALE_REFERENCE, NULL, 0);
    }
    if (slot->type_id == 0 || limit == 0 || slot->generation > limit) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    if (slot->generation == limit) {
        slot->retired = true;
        return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0);
    }
    slot->generation++;
    return ama_status_success();
}

AmaStatus ama_slot_advance_generation(AmaSlotIdentity *slot) {
    return ama_slot_advance_generation_limited(slot, UINT64_MAX);
}
