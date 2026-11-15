/** @file
 * @brief Cancel exactly one native registration while preserving a newer wait and separate storage lifetime.
 * [ASYNC-09](../../../../spec/language/async-and-parallelism.md#async-09)
 * invalidates visible task waits before detachment; [PAR-04](../../../../spec/language/async-and-parallelism.md#par-04)
 * keeps pending native state alive through shutdown and acknowledgment.
 */
#include "../internal.h"

/** Full identity comparison prevents a late cancel from clearing a replacement registration. */
AmaStatus ama_channel_wait_cancel(void *state, AmaNativeEventToken token, bool sending, bool *retired) {
    if (retired == NULL) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    AmaNativeChannel *channel = state;
    AmaNativeEventSubmission event = {0};
    ama_channel_lock(channel);
    AmaNativeEventSubmission *slot = sending ? &channel->sending.event : &channel->receiving.event;
    *retired = slot->state != NULL && ama_native_event_token_equal(slot->token, token);
    if (*retired) {
        event = *slot;
        *slot = (AmaNativeEventSubmission){0};
        if (sending) { channel->sending.required_bytes = 0; }
    }
    ama_channel_unlock(channel);
    ama_native_event_submission_drop(&event);
    return ama_status_success();
}

/** A delivered/cancelled handle still owns service/code storage until this acknowledgment. */
void ama_channel_wait_drop(void *state, AmaNativeEventToken token, bool sending) {
    bool retired = false;
    (void)ama_channel_wait_cancel(state, token, sending, &retired);
    ama_channel_release(state);
}
/** Immutable creator dispatch remains alive while any registration credit exists. */
const AmaNativeChannelWaitCallbacks ama_channel_wait_callbacks = {ama_channel_wait_cancel, ama_channel_wait_drop};
