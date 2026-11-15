/** @file
 * @brief Keep fixed queue bookkeeping and independent native endpoint lifetimes behind one short gate.
 * [PAR-04](../../../spec/language/async-and-parallelism.md#par-04) bounds
 * accepted count/bytes; [PAR-05](../../../spec/language/async-and-parallelism.md#par-05)
 * permits synchronized native services without sharing mutable Ama state.
 */
#ifndef AMA_NATIVE_CHANNELS_INTERNAL_H
#define AMA_NATIVE_CHANNELS_INTERNAL_H
#include <stdatomic.h>
#include <string.h>
#include "channel.h"

/** One private immutable schema and fixed ring, kept alive by two logical ports.
 * All mutable metadata uses locked; each nonempty cell owns exactly one envelope.
 */
typedef struct {
    _Atomic bool locked;
    size_t references, capacity, maximum_bytes, count, bytes, head, tail;
    bool sender_closed, aborted;
    AmaNativeMessageSchema schema;
    unsigned char *name;
    AmaNativeMessageField *fields;
    AmaNativeMessageBytes *cells;
    AmaNativeCodeLease code;
} AmaNativeChannel;

/** Wait for finite native metadata only; execute no allocator, callback or Ama under this gate. */
void ama_channel_lock(AmaNativeChannel *);
/** Publish metadata changes before the opposite endpoint observes them. */
void ama_channel_unlock(AmaNativeChannel *);
/** Ordinary channel result without allocation or technical failure. */
static inline AmaNativeChannelResult ama_channel_result(AmaNativeChannelReason reason) {
    return (AmaNativeChannelResult){ama_status_success(), reason, AMA_MESSAGE_ACCEPTED};
}
/** Configuration/ownership failure is technical, never inferred from a channel reason. */
static inline AmaNativeChannelResult ama_channel_failure(AmaStatus status) {
    return (AmaNativeChannelResult){status, AMA_CHANNEL_EMPTY, AMA_MESSAGE_ACCEPTED};
}
/** An output owner is entirely empty, including its native code credit. */
static inline bool ama_channel_empty_owner(const AmaNativeMessageBytes *value) {
    return value != NULL && value->data == NULL && value->length == 0
        && value->code.resource == NULL && value->code.release == NULL;
}
/** Advance a valid ring locator without wraparound arithmetic. */
static inline size_t ama_channel_next(size_t index, size_t capacity) { return index == capacity - 1 ? 0 : index + 1; }
/** Creator-side send/take/close/abort operations; caller retains its unique logical port. */
AmaNativeChannelResult ama_channel_send(void *, AmaNativeMessageBytes *);
/** Move one head owner under the metadata gate without decoding it. */
AmaNativeChannelResult ama_channel_take(void *, AmaNativeMessageBytes *);
/** Close producer admission, preserving already accepted data. */
AmaStatus ama_channel_close(void *);
/** Abort atomically before disposing detached queue owners. */
AmaStatus ama_channel_abort(void *);
/** Consume one port credit after its close/abort operation; final retirement runs outside the gate. */
void ama_channel_release(AmaNativeChannel *);
/** Creator callbacks keep internal service layout private across component images. */
extern const AmaNativeChannelSendCallbacks ama_channel_sender_callbacks;
/** Independent receiver vtable; neither table grants execution or Ama state access. */
extern const AmaNativeChannelReceiveCallbacks ama_channel_receiver_callbacks;
#endif
