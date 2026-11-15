/** @file
 * @brief Borrow immutable bytes bytes and replace complete native values atomically.
 * [COPY-02](../../../../spec/language/values-and-references.md#copy-02)
 * preserves the old destination through preparation failure; immutable backing
 * is never edited during replacement, including aliased source byte views.
 */
#include "internal.h"

/** Publish a read-only counted view only for a live value; no ownership moves. */
AmaStatus AMA_NATIVE(view)(const AMA_NATIVE(State) *state, AmaByteView *output) {
    if (!state->live) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    *output = state->backing == NULL ? (AmaByteView){0}
        : (AmaByteView){state->backing->bytes, state->backing->length};
    return ama_status_success();
}

/** Derive nonempty truth from a valid borrowed view, preserving output on failure. */
AmaStatus AMA_NATIVE(to_condition)(const AMA_NATIVE(State) *state, bool *output) {
    AmaByteView bytes;
    AmaStatus status = AMA_NATIVE(view)(state, &bytes);
    if (status.code == AMA_OK) { *output = bytes.length != 0; }
    return status;
}

/** Prepare independently before releasing the old live credit, then transfer the
 * prepared value. An aliased byte source remains valid throughout preparation.
 */
AmaStatus AMA_NATIVE(replace)(AMA_NATIVE(State) *destination, AmaByteView bytes) {
    if (!destination->live) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    AMA_NATIVE(State) prepared;
    AmaStatus status = AMA_NATIVE(from_literal)(&prepared, bytes);
    if (status.code != AMA_OK) { return status; }
    status = AMA_NATIVE(destroy)(destination);
    if (status.code != AMA_OK) { (void)AMA_NATIVE(destroy)(&prepared); return status; }
    *destination = prepared;
    return ama_status_success();
}
