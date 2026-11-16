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

/** One private immutable schema and fixed ring, kept alive by ports and registration owners.
 * All mutable metadata uses locked; each nonempty cell owns exactly one envelope.
 * Component contract: [PAR-04](../../../spec/language/async-and-parallelism.md#par-04), [PAR-05](../../../spec/language/async-and-parallelism.md#par-05). */
typedef struct {
    _Atomic bool locked;
    size_t references, capacity, maximum_bytes, count, bytes, head, tail;
    bool sender_closed, aborted;
    AmaNativeMessageSchema schema;
    unsigned char *name;
    AmaNativeMessageField *fields;
    AmaNativeMessageBytes *cells;
    AmaNativeCodeLease code;
    /** Single-shot native subscriptions own event rights, not task identities or Ama views. Component contract: [PAR-04](../../../spec/language/async-and-parallelism.md#par-04), [PAR-05](../../../spec/language/async-and-parallelism.md#par-05). */
    struct { AmaNativeEventSubmission event; size_t required_bytes; } sending, receiving;
} AmaNativeChannel;

/** Detach at most one notification per direction under the gate, then publish outside it. Component contract: [PAR-04](../../../spec/language/async-and-parallelism.md#par-04), [PAR-05](../../../spec/language/async-and-parallelism.md#par-05). */
typedef struct { AmaNativeEventSubmission sending, receiving; } AmaChannelSignals;

/** Wait for finite native metadata only; execute no allocator, callback or Ama under this gate. Component contract: [PAR-04](../../../spec/language/async-and-parallelism.md#par-04), [PAR-05](../../../spec/language/async-and-parallelism.md#par-05). */
void ama_channel_lock(AmaNativeChannel *);
/** Publish metadata changes before the opposite endpoint observes them. Component contract: [PAR-04](../../../spec/language/async-and-parallelism.md#par-04), [PAR-05](../../../spec/language/async-and-parallelism.md#par-05). */
void ama_channel_unlock(AmaNativeChannel *);
/** Ordinary channel result without allocation or technical failure. Component contract: [PAR-04](../../../spec/language/async-and-parallelism.md#par-04), [PAR-05](../../../spec/language/async-and-parallelism.md#par-05). */
static inline AmaNativeChannelResult ama_channel_result(AmaNativeChannelReason reason) {
    return (AmaNativeChannelResult){ama_status_success(), reason, AMA_MESSAGE_ACCEPTED};
}
/** Configuration/ownership failure is technical, never inferred from a channel reason. Component contract: [PAR-04](../../../spec/language/async-and-parallelism.md#par-04), [PAR-05](../../../spec/language/async-and-parallelism.md#par-05). */
static inline AmaNativeChannelResult ama_channel_failure(AmaStatus status) {
    return (AmaNativeChannelResult){status, AMA_CHANNEL_EMPTY, AMA_MESSAGE_ACCEPTED};
}
/** An output owner is entirely empty, including its native code credit. Component contract: [PAR-04](../../../spec/language/async-and-parallelism.md#par-04), [PAR-05](../../../spec/language/async-and-parallelism.md#par-05). */
static inline bool ama_channel_empty_owner(const AmaNativeMessageBytes *value) {
    return value != NULL && value->data == NULL && value->length == 0
        && value->code.resource == NULL && value->code.release == NULL;
}
/** Advance a valid ring locator without wraparound arithmetic. Component contract: [PAR-04](../../../spec/language/async-and-parallelism.md#par-04), [PAR-05](../../../spec/language/async-and-parallelism.md#par-05). */
static inline size_t ama_channel_next(size_t index, size_t capacity) { return index == capacity - 1 ? 0 : index + 1; }
/** Creator-side send/take/close/abort operations; caller retains its unique logical port. Component contract: [PAR-04](../../../spec/language/async-and-parallelism.md#par-04), [PAR-05](../../../spec/language/async-and-parallelism.md#par-05). */
AmaNativeChannelResult ama_channel_send(void *, AmaNativeMessageBytes *);
/** Move one head owner under the metadata gate without decoding it. Component contract: [PAR-04](../../../spec/language/async-and-parallelism.md#par-04), [PAR-05](../../../spec/language/async-and-parallelism.md#par-05). */
AmaNativeChannelResult ama_channel_take(void *, AmaNativeMessageBytes *);
/** Close producer admission, preserving already accepted data. Component contract: [PAR-04](../../../spec/language/async-and-parallelism.md#par-04), [PAR-05](../../../spec/language/async-and-parallelism.md#par-05). */
AmaStatus ama_channel_close(void *);
/** Abort atomically before disposing detached queue owners. Component contract: [PAR-04](../../../spec/language/async-and-parallelism.md#par-04), [PAR-05](../../../spec/language/async-and-parallelism.md#par-05). */
AmaStatus ama_channel_abort(void *);
/** Consume one endpoint/registration credit after its close/abort/cancel operation;
 * final retirement runs outside the gate.
 * Component contract: [PAR-04](../../../spec/language/async-and-parallelism.md#par-04), [PAR-05](../../../spec/language/async-and-parallelism.md#par-05). */
void ama_channel_release(AmaNativeChannel *);
/** Creator callbacks keep internal service layout private across component images. Component contract: [PAR-04](../../../spec/language/async-and-parallelism.md#par-04), [PAR-05](../../../spec/language/async-and-parallelism.md#par-05). */
extern const AmaNativeChannelSendCallbacks ama_channel_sender_callbacks;
/** Independent receiver vtable; neither table grants execution or Ama state access. Component contract: [PAR-04](../../../spec/language/async-and-parallelism.md#par-04), [PAR-05](../../../spec/language/async-and-parallelism.md#par-05). */
extern const AmaNativeChannelReceiveCallbacks ama_channel_receiver_callbacks;
/** Install an exact sender wait or acknowledge readiness without keeping a redundant handle. Component contract: [PAR-04](../../../spec/language/async-and-parallelism.md#par-04), [PAR-05](../../../spec/language/async-and-parallelism.md#par-05). */
AmaStatus ama_channel_wait_send(void *, size_t, const AmaNativeEventSubmission *, AmaNativeChannelWait *);
/** Install the matching sole receive wait. Component contract: [PAR-04](../../../spec/language/async-and-parallelism.md#par-04), [PAR-05](../../../spec/language/async-and-parallelism.md#par-05). */
AmaStatus ama_channel_wait_receive(void *, const AmaNativeEventSubmission *, AmaNativeChannelWait *);
/** Check exact predicates and move ready subscriptions; caller already holds the gate. Component contract: [PAR-04](../../../spec/language/async-and-parallelism.md#par-04), [PAR-05](../../../spec/language/async-and-parallelism.md#par-05). */
void ama_channel_collect_signals(AmaNativeChannel *, AmaChannelSignals *);
/** Publish detached notifications only outside every native service gate. Component contract: [PAR-04](../../../spec/language/async-and-parallelism.md#par-04), [PAR-05](../../../spec/language/async-and-parallelism.md#par-05). */
void ama_channel_publish_signals(AmaChannelSignals *);
/** Compare the full key before detaching publication; release event rights after unlock. Component contract: [PAR-04](../../../spec/language/async-and-parallelism.md#par-04), [PAR-05](../../../spec/language/async-and-parallelism.md#par-05). */
AmaStatus ama_channel_wait_cancel(void *, AmaNativeEventToken, bool, bool *);
/** Consume cancellation and its independent native service/code storage credit. Component contract: [PAR-04](../../../spec/language/async-and-parallelism.md#par-04), [PAR-05](../../../spec/language/async-and-parallelism.md#par-05). */
void ama_channel_wait_drop(void *, AmaNativeEventToken, bool);
/** Ready capacity includes terminal/impossible outcomes, which the awakened executor rechecks. Component contract: [PAR-04](../../../spec/language/async-and-parallelism.md#par-04), [PAR-05](../../../spec/language/async-and-parallelism.md#par-05). */
bool ama_channel_sender_ready(const AmaNativeChannel *, size_t);
/** Native wait callbacks are creator-image operations retained through the last registration owner. Component contract: [PAR-04](../../../spec/language/async-and-parallelism.md#par-04), [PAR-05](../../../spec/language/async-and-parallelism.md#par-05). */
extern const AmaNativeChannelWaitCallbacks ama_channel_wait_callbacks;
#endif
