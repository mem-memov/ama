/** @file
 * @brief Invoke creator-image channel operations through move-only native endpoint rights.
 * [PAR-04](../../../../spec/language/async-and-parallelism.md#par-04) assigns
 * distinct sender/receiver shutdown; [EMBED-06](../../../../spec/implementation/c-library.md#embed-06)
 * keeps callback code retained through final native endpoint retirement.
 */
#include "../internal.h"

/** The sender's implicit lifecycle action is normal close, preserving accepted data. Component contract: [PAR-04](../../../../spec/language/async-and-parallelism.md#par-04), [EMBED-06](../../../../spec/implementation/c-library.md#embed-06). */
static void channel_sender_release(void *state) {
    (void)ama_channel_close(state);
    ama_channel_release(state);
}
/** The receiver's implicit lifecycle action aborts before releasing its native credit. Component contract: [PAR-04](../../../../spec/language/async-and-parallelism.md#par-04), [EMBED-06](../../../../spec/implementation/c-library.md#embed-06). */
static void channel_receiver_release(void *state) {
    (void)ama_channel_abort(state);
    ama_channel_release(state);
}
/** Each creator table is immutable and contains no context-specific mutable state. Component contract: [PAR-04](../../../../spec/language/async-and-parallelism.md#par-04), [EMBED-06](../../../../spec/implementation/c-library.md#embed-06). */
const AmaNativeChannelSendCallbacks ama_channel_sender_callbacks = {
    .send = ama_channel_send, .close = ama_channel_close, .drop = channel_sender_release, .wait = ama_channel_wait_send
};
/** Separate operation signatures prevent treating receipt as producer ownership. Component contract: [PAR-04](../../../../spec/language/async-and-parallelism.md#par-04), [EMBED-06](../../../../spec/implementation/c-library.md#embed-06). */
const AmaNativeChannelReceiveCallbacks ama_channel_receiver_callbacks = {
    .take = ama_channel_take, .abort = ama_channel_abort, .drop = channel_receiver_release, .wait = ama_channel_wait_receive
};

/** Empty/moved endpoint requests fail without consulting native service storage. Component contract: [PAR-04](../../../../spec/language/async-and-parallelism.md#par-04), [EMBED-06](../../../../spec/implementation/c-library.md#embed-06). */
AmaNativeChannelResult ama_native_channel_send(const AmaNativeChannelSender *sender, AmaNativeMessageBytes *message) {
    if (sender == NULL || sender->state == NULL || sender->callbacks == NULL) {
        return ama_channel_failure(ama_status_make(AMA_INVALID_STATE, NULL, 0));
    }
    return sender->callbacks->send(sender->state, message);
}
/** Dispatch only after validating the native endpoint shape; the creator checks the empty destination. Component contract: [PAR-04](../../../../spec/language/async-and-parallelism.md#par-04), [EMBED-06](../../../../spec/implementation/c-library.md#embed-06). */
AmaNativeChannelResult ama_native_channel_take(const AmaNativeChannelReceiver *receiver, AmaNativeMessageBytes *output) {
    if (receiver == NULL || receiver->state == NULL || receiver->callbacks == NULL) {
        return ama_channel_failure(ama_status_make(AMA_INVALID_STATE, NULL, 0));
    }
    return receiver->callbacks->take(receiver->state, output);
}
/** Repeated close of a live port is harmless; moved/absent ports cannot address a service. Component contract: [PAR-04](../../../../spec/language/async-and-parallelism.md#par-04), [EMBED-06](../../../../spec/implementation/c-library.md#embed-06). */
AmaStatus ama_native_channel_close(const AmaNativeChannelSender *sender) {
    if (sender == NULL || sender->state == NULL || sender->callbacks == NULL) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    return sender->callbacks->close(sender->state);
}
/** Abort does not consume the endpoint; its later drop remains exact and idempotent. Component contract: [PAR-04](../../../../spec/language/async-and-parallelism.md#par-04), [EMBED-06](../../../../spec/implementation/c-library.md#embed-06). */
AmaStatus ama_native_channel_abort(const AmaNativeChannelReceiver *receiver) {
    if (receiver == NULL || receiver->state == NULL || receiver->callbacks == NULL) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    return receiver->callbacks->abort(receiver->state);
}
/** Clear the C owner before invoking its final creator-code operation. Component contract: [PAR-04](../../../../spec/language/async-and-parallelism.md#par-04), [EMBED-06](../../../../spec/implementation/c-library.md#embed-06). */
void ama_native_channel_sender_drop(AmaNativeChannelSender *owner) {
    if (owner == NULL) { return; }
    AmaNativeChannelSender sender = *owner;
    *owner = (AmaNativeChannelSender){0};
    if (sender.state != NULL) { sender.callbacks->drop(sender.state); }
}
/** No queued owner can survive receiver disposal unless already taken independently. Component contract: [PAR-04](../../../../spec/language/async-and-parallelism.md#par-04), [EMBED-06](../../../../spec/implementation/c-library.md#embed-06). */
void ama_native_channel_receiver_drop(AmaNativeChannelReceiver *owner) {
    if (owner == NULL) { return; }
    AmaNativeChannelReceiver receiver = *owner;
    *owner = (AmaNativeChannelReceiver){0};
    if (receiver.state != NULL) { receiver.callbacks->drop(receiver.state); }
}
