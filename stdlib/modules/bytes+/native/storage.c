/** @file
 * @brief Construct immutable bytes and maintain exact native share credits.
 * [PROTO-06](../../../../spec/implementation/native-interface.md#proto-06)
 * makes clone O(1) while preserving independent values; final destruction
 * consumes one credit and never edits another value's immutable bytes.
 */
#include "internal.h"
#include <string.h>

/** Empty bytes needs no backing allocation but still has a live native credit. */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *destination) {
    *destination = (AMA_NATIVE(State)){.live = true};
    return ama_status_success();
}

/** Validate before allocation; failure leaves no native destruction obligation. */
AmaStatus AMA_NATIVE(from_literal)(AMA_NATIVE(State) *destination, AmaByteView bytes) {
    *destination = (AMA_NATIVE(State)){0};
    if (bytes.data == NULL && bytes.length != 0) { return ama_status_make(AMA_INVALID_LITERAL, NULL, 0); }
    if (bytes.length > PTRDIFF_MAX || bytes.length > SIZE_MAX - sizeof(struct AMA_NATIVE(Backing))) {
        return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0);
    }
    if (bytes.length == 0) { destination->live = true; return ama_status_success(); }
    struct AMA_NATIVE(Backing) *backing = AMA_BYTES_ALLOCATE(sizeof(*backing) + bytes.length);
    if (backing == NULL) { return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0); }
    backing->shares = 1;
    backing->length = bytes.length;
    memcpy(backing->bytes, bytes.data, bytes.length);
    *destination = (AMA_NATIVE(State)){.backing = backing, .live = true};
    return ama_status_success();
}

/** A checked increment grants the destination's credit before publication.
 * Source and failed destination remain independent; no wrapped count is visible.
 */
AmaStatus AMA_NATIVE(clone)(AMA_NATIVE(State) *destination, const AMA_NATIVE(State) *source) {
    if (destination == source) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    *destination = (AMA_NATIVE(State)){0};
    if (!source->live) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    if (source->backing != NULL) {
        size_t shares = source->backing->shares;
        if (shares == 0) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
        if (shares >= AMA_BYTES_SHARE_LIMIT) { return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0); }
        source->backing->shares++;
    }
    *destination = *source;
    return ama_status_success();
}

/** The last live credit releases backing once; repeated finalization refuses. */
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) {
    if (!state->live || (state->backing != NULL && state->backing->shares == 0)) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    if (state->backing != NULL && --state->backing->shares == 0) { AMA_BYTES_RELEASE(state->backing); }
    *state = (AMA_NATIVE(State)){0};
    return ama_status_success();
}
