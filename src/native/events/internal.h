/** @file
 * @brief Separate native queue storage, reference lifetime and identity validation from Ama state.
 * [ASYNC-05](../../../spec/language/async-and-parallelism.md#async-05) forbids
 * retained Ama handles/views; these cells contain only native tokens and byte owners.
 */
#ifndef AMA_NATIVE_EVENTS_INTERNAL_H
#define AMA_NATIVE_EVENTS_INTERNAL_H
#include <stdatomic.h>
#include <semaphore.h>
#include "native/events/event.h"
#define AMA_NATIVE_EVENT_NONE SIZE_MAX
#define AMA_NATIVE_EVENT_ERROR(code, text) ama_status_make((code), (text), sizeof(text) - 1)
/** Partition each fixed native locator into free, reserved, armed or retained completion. */
typedef enum { AMA_EVENT_FREE, AMA_EVENT_PREPARED, AMA_EVENT_ARMED, AMA_EVENT_COMPLETED } AmaNativeEventState;
/** Next links the free stack or completion queue; previous is used only while completed. */
typedef struct {
    AmaNativeEventState state;
    uint64_t generation;
    size_t next, previous;
    int64_t code;
    AmaNativeEventBuffer buffer;
} AmaNativeEventCell;
/** Mutable native metadata only. Unique owner performs lifecycle/consumer operations;
 * separately owning callbacks serialize publication/refcounts through the short metadata gate.
 */
struct AmaNativeEventService {
    _Atomic bool locked;
    bool closed, notified;
    sem_t wake;
    size_t references, capacity, free, first, last, armed;
    uint64_t generation;
    AmaNativeEventToken identity;
    AmaNativeEventCell *cells;
    AmaNativeEventKeepalive keepalive;
};
/** Initialize one process-private binary notification resource before service publication. */
AmaStatus ama_native_event_wake_initialize(AmaNativeEventService *);
/** Retire the initialized wake resource after the final lease and serialized waiter have left. */
void ama_native_event_wake_destroy(AmaNativeEventService *);
/** Publish a reserved binary wake credit outside the metadata gate, retaining no Ama state. */
void ama_native_event_notify(AmaNativeEventService *);
/** Prepare an exact uint64 successor with an inclusive bound, preserving output on failure. */
bool ama_native_event_sequence(uint64_t, uint64_t, uint64_t *);
/** Issue a fresh process-lifetime domain/service pair; no native record address is its identity. */
AmaStatus ama_native_event_identity(AmaNativeEventToken *);
/** Acquire/release finite native metadata sections, never invoking allocators or user callbacks. */
void ama_native_event_lock(AmaNativeEventService *);
/** Publish metadata before another producer or owner enters its section. */
void ama_native_event_unlock(AmaNativeEventService *);
/** Retain a native resource reference, including closed services, without wrapping. */
bool ama_native_event_retain(void *);
/** Consume one reference; final closed resources and keepalive retire outside the gate. */
void ama_native_event_release(void *);
/** Obtain the creator-image operation table for a newly prepared callback lease. */
const AmaNativeEventCallbacks *ama_native_event_callbacks(void);
/** Resolve an exact live locator while the service gate is held; closed services resolve nothing. */
AmaNativeEventCell *ama_native_event_cell(AmaNativeEventService *, AmaNativeEventToken);
/** Return an unlinked cell to its free stack while holding the metadata gate. */
void ama_native_event_recycle(AmaNativeEventService *, size_t);
/** Copy into creator-owned bytes; the caller retains an independent service lease throughout. */
AmaStatus ama_native_event_copy(void *, AmaByteView, AmaNativeEventBuffer *);
/** Recognize only this service's valid native byte ownership or the canonical empty buffer. */
bool ama_native_event_buffer_valid(AmaNativeEventService *, const AmaNativeEventBuffer *);
/** Release the immutable byte allocation and its service dependency outside every gate. */
void ama_native_event_buffer_release(void *);
/** Queue one authenticated native completion with no allocation or Ama execution. */
AmaNativeEventPostResult ama_native_event_complete(void *, AmaNativeEventToken, int64_t, AmaNativeEventBuffer *);
/** Read retained readiness under the native metadata gate. */
AmaStatus ama_native_event_check(void *, AmaNativeEventToken, bool *);
/** Classify exact token post admission without mutation or an unarmed registration loophole. */
AmaNativeEventAcceptance ama_native_event_accept(void *, AmaNativeEventToken);
#endif
