/** @file
 * @brief Dispatch native wait registration/cancellation through the owning creator image.
 * [PAR-04](../../../../spec/language/async-and-parallelism.md#par-04) exposes
 * backpressure readiness without blocking an Ama executor or exporting a borrowed view.
 */
#include "../internal.h"

/** Port shape is checked before invoking the retained creator operation. */
AmaStatus ama_native_channel_wait_send(const AmaNativeChannelSender *sender, size_t required,
    const AmaNativeEventSubmission *source, AmaNativeChannelWait *output) {
    if (sender == NULL || sender->state == NULL || sender->callbacks == NULL || sender->callbacks->wait == NULL) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    return sender->callbacks->wait(sender->state, required, source, output);
}
/** The native event source grants notification only, never execution authority. */
AmaStatus ama_native_channel_wait_receive(const AmaNativeChannelReceiver *receiver,
    const AmaNativeEventSubmission *source, AmaNativeChannelWait *output) {
    if (receiver == NULL || receiver->state == NULL || receiver->callbacks == NULL || receiver->callbacks->wait == NULL) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    return receiver->callbacks->wait(receiver->state, source, output);
}
/** An empty acknowledged owner has no publication to cancel; success reports false. */
AmaStatus ama_native_channel_wait_cancel(const AmaNativeChannelWait *owner, bool *retired) {
    if (owner == NULL || retired == NULL) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    if (owner->state == NULL) { *retired = false; return ama_status_success(); }
    if (owner->callbacks == NULL || owner->callbacks->cancel == NULL) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    return owner->callbacks->cancel(owner->state, owner->token, owner->sender, retired);
}
/** Consume the C owner before final creator-side native cancellation/storage release. */
void ama_native_channel_wait_drop(AmaNativeChannelWait *owner) {
    if (owner == NULL) { return; }
    AmaNativeChannelWait value = *owner;
    *owner = (AmaNativeChannelWait){0};
    if (value.state != NULL) { value.callbacks->drop(value.state, value.token, value.sender); }
}
