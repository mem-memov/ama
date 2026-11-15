/** @file
 * @brief Atomically accept one complete valid envelope or preserve its sender ownership unchanged.
 * [PAR-04](../../../../spec/language/async-and-parallelism.md#par-04) bounds
 * both live message count and encoded bytes without partial enqueue or silent loss.
 */
#include "../internal.h"

/** Validation borrows immutable schema/data outside the gate. The final gate
 * rechecks closure and capacities, establishing enqueue's linearization point.
 * The caller owns its logical sender throughout; the receiver may act concurrently.
 */
AmaNativeChannelResult ama_channel_send(void *state, AmaNativeMessageBytes *message) {
    AmaNativeChannel *channel = state;
    if (message == NULL || (message->data == NULL && message->length != 0)
        || ((message->code.resource == NULL) != (message->code.release == NULL))) {
        return ama_channel_failure(ama_status_make(AMA_INVALID_STATE, NULL, 0));
    }
    if (message->length > channel->maximum_bytes) { return ama_channel_result(AMA_CHANNEL_TOO_LARGE); }
    AmaNativeMessageResult validated = ama_native_message_validate(&channel->schema,
        (AmaByteView){message->data, message->length});
    if (validated.status.code != AMA_OK) { return ama_channel_failure(validated.status); }
    if (validated.reason != AMA_MESSAGE_ACCEPTED) {
        AmaNativeChannelResult result = ama_channel_result(AMA_CHANNEL_SCHEMA_REJECTED);
        result.message_reason = validated.reason;
        return result;
    }
    AmaChannelSignals signals = {0};
    ama_channel_lock(channel);
    AmaNativeChannelResult result = ama_channel_result(AMA_CHANNEL_ACCEPTED);
    if (channel->aborted) { result.reason = AMA_CHANNEL_ABORTED; }
    else if (channel->sender_closed) { result.reason = AMA_CHANNEL_CLOSED; }
    else if (channel->count == channel->capacity || message->length > channel->maximum_bytes - channel->bytes) {
        result.reason = AMA_CHANNEL_FULL;
    } else {
        size_t length = message->length;
        channel->cells[channel->tail] = *message;
        *message = (AmaNativeMessageBytes){0};
        channel->tail = ama_channel_next(channel->tail, channel->capacity);
        channel->count++;
        channel->bytes += length;
    }
    ama_channel_collect_signals(channel, &signals);
    ama_channel_unlock(channel);
    ama_channel_publish_signals(&signals);
    return result;
}
