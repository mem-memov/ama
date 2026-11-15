/** @file
 * @brief Own native submissions and retained completions without any Ama execution capability.
 * [ASYNC-05](../../../spec/language/async-and-parallelism.md#async-05) gives
 * callbacks native resources and immutable tokens; only the executor imports data.
 */
#ifndef AMA_NATIVE_EVENTS_EVENT_H
#define AMA_NATIVE_EVENTS_EVENT_H
#include "native/abi/bytes.h"
#include "native/status/status.h"

/** Native service owner; no Ama instance, view, handle or saved position is stored here. */
typedef struct AmaNativeEventService AmaNativeEventService;
/** Complete native identity. Slot is only a locator; domain/service/generation authenticate it. */
typedef struct {
    const void *domain;
    uint64_t service, generation;
    size_t slot;
} AmaNativeEventToken;
/** Move-only immutable native bytes, with their originating release operation. */
typedef struct AmaNativeEventBuffer {
    const unsigned char *data;
    size_t length;
    void *owner;
    void (*release)(void *);
} AmaNativeEventBuffer;
/** Native completion admission; every rejection leaves the supplied buffer owned by its caller. */
typedef enum {
    AMA_EVENT_QUEUED, AMA_EVENT_DUPLICATE, AMA_EVENT_STALE,
    AMA_EVENT_CLOSED, AMA_EVENT_UNARMED, AMA_EVENT_INVALID_BUFFER
} AmaNativeEventPostResult;
/** Creator-image operations let external C workers use the correct native service implementation. */
typedef struct {
    bool (*retain)(void *);
    void (*drop)(void *);
    AmaStatus (*copy)(void *, AmaByteView, AmaNativeEventBuffer *);
    AmaNativeEventPostResult (*post)(void *, AmaNativeEventToken, int64_t, AmaNativeEventBuffer *);
    AmaStatus (*ready)(void *, AmaNativeEventToken, bool *);
} AmaNativeEventCallbacks;
/** Move-only callback lease. State is a retained native resource, never borrowed Ama storage. */
typedef struct {
    void *state;
    const AmaNativeEventCallbacks *callbacks;
    AmaNativeEventToken token;
} AmaNativeEventSubmission;
/** Move-only completion returned to the sole native consumer. Code is operation-specific native data. */
typedef struct {
    AmaNativeEventToken token;
    int64_t code;
    AmaNativeEventBuffer buffer;
} AmaNativeEventCompletion;
/** Optional native code/resource lease transferred on successful service creation.
 * Its resource must outlive all callbacks/buffers and must not contain borrowed Ama state.
 * Its release is infallible native retirement and must not enter Ama.
 * The loader's call barrier covers its final release just as it covers other C API releases.
 */
typedef struct {
    void *resource;
    void (*release)(void *);
} AmaNativeEventKeepalive;
/** A synchronous C submission routine. It may post immediately; retained jobs first clone the
 * borrowed submission. Submission errors are posted as native completion data, including no payload.
 */
typedef void (*AmaNativeEventStart)(const AmaNativeEventSubmission *, void *native_resource);

/** Create one unique native owner with positive fixed capacity. Success moves keepalive;
 * failure preserves it and leaves the initially null output unchanged.
 */
AmaStatus ama_native_event_service_create(size_t, AmaNativeEventKeepalive *, AmaNativeEventService **);
/** Retain the native service for a method resource. Output must be empty; failure
 * preserves it. This lease contains no Ama storage and does not reopen a closed service.
 */
AmaStatus ama_native_event_service_lease(AmaNativeEventService *, AmaNativeEventKeepalive *);
/** Obtain callback rights for an immutable supplied token without reserving another cell.
 * Token authentication remains the responsibility of post/ready/arm; stale notifications
 * therefore retain their original identity and are rejected normally. Output must be empty.
 */
AmaStatus ama_native_event_service_callback(AmaNativeEventService *, AmaNativeEventToken, AmaNativeEventSubmission *);
/** Reserve one cell and immutable token before publishing an Ama registration. Busy capacity,
 * exhausted identity/refcounts and closed owners change nothing; the output must be empty.
 */
AmaStatus ama_native_event_service_prepare(AmaNativeEventService *, AmaNativeEventSubmission *);
/** Mark the reserved cell armed before invoking the borrowed submit routine outside every gate.
 * The owner serializes arm/take/dispose; callbacks may post concurrently and never execute Ama.
 */
AmaStatus ama_native_event_service_arm(AmaNativeEventService *, const AmaNativeEventSubmission *, AmaNativeEventStart, void *);
/** Retire only a never-armed reservation; reject other states without changing them. */
AmaStatus ama_native_event_service_discard(AmaNativeEventService *, AmaNativeEventToken);
/** Move the oldest accepted completion into an empty output and recycle its locator. No allocation
 * occurs. Empty queues report available=false; only the owner performs consumer operations.
 */
AmaStatus ama_native_event_service_take(AmaNativeEventService *, AmaNativeEventCompletion *, bool *available);
/** Close before dropping the unique owner. Queued buffers/cells retire immediately; callback
 * and byte owners retain the native service/code lease until their own acknowledgments/releases.
 */
void ama_native_event_service_dispose(AmaNativeEventService **);

/** Compare complete identities without interpreting a native address as an Ama identity. */
static inline bool ama_native_event_token_equal(AmaNativeEventToken left, AmaNativeEventToken right) {
    return left.domain == right.domain && left.service == right.service
        && left.generation == right.generation && left.slot == right.slot;
}
/** Clone callback rights explicitly; plain C copying does not create another owning lease. */
static inline AmaStatus ama_native_event_submission_clone(const AmaNativeEventSubmission *source, AmaNativeEventSubmission *output) {
    if (source == NULL || source->state == NULL || output == NULL || output->state != NULL) {
        return ama_status_make(AMA_INVALID_STATE, "Invalid native submission clone", sizeof("Invalid native submission clone") - 1);
    }
    if (!source->callbacks->retain(source->state)) {
        return ama_status_make(AMA_LIMIT_EXCEEDED, "Native callback references exhausted", sizeof("Native callback references exhausted") - 1);
    }
    *output = *source;
    return ama_status_success();
}
/** Consume one callback lease after its final callback has returned; empty is harmless. */
static inline void ama_native_event_submission_drop(AmaNativeEventSubmission *submission) {
    if (submission != NULL) {
        AmaNativeEventSubmission owner = *submission;
        *submission = (AmaNativeEventSubmission){0};
        if (owner.state != NULL) { owner.callbacks->drop(owner.state); }
    }
}
/** Copy borrowed bytes synchronously into independent native ownership. No borrowed view escapes. */
static inline AmaStatus ama_native_event_buffer_copy(const AmaNativeEventSubmission *submission, AmaByteView source, AmaNativeEventBuffer *output) {
    if (submission == NULL || submission->state == NULL) {
        return ama_status_make(AMA_INVALID_STATE, "Empty native callback lease", sizeof("Empty native callback lease") - 1);
    }
    return submission->callbacks->copy(submission->state, source, output);
}
/** Release immutable native bytes through their originating implementation, then leave empty. */
static inline void ama_native_event_buffer_drop(AmaNativeEventBuffer *buffer) {
    if (buffer != NULL) {
        AmaNativeEventBuffer owner = *buffer;
        *buffer = (AmaNativeEventBuffer){0};
        if (owner.owner != NULL) { owner.release(owner.owner); }
    }
}
/** Publish through the creator's native queue. Queued moves buffer; rejection preserves ownership. */
static inline AmaNativeEventPostResult ama_native_event_post(const AmaNativeEventSubmission *submission, int64_t code, AmaNativeEventBuffer *buffer) {
    if (submission == NULL || submission->state == NULL) { return AMA_EVENT_STALE; }
    return submission->callbacks->post(submission->state, submission->token, code, buffer);
}
/** Recheck retained native completion using the full token; this accesses no Ama state. */
static inline AmaStatus ama_native_event_ready(const AmaNativeEventSubmission *submission, bool *ready) {
    if (submission == NULL || submission->state == NULL || ready == NULL) {
        return ama_status_make(AMA_INVALID_STATE, "Invalid native readiness query", sizeof("Invalid native readiness query") - 1);
    }
    return submission->callbacks->ready(submission->state, submission->token, ready);
}
/** Release a consumed completion's byte owner and empty its scalar metadata. */
static inline void ama_native_event_completion_drop(AmaNativeEventCompletion *completion) {
    if (completion != NULL) {
        ama_native_event_buffer_drop(&completion->buffer);
        *completion = (AmaNativeEventCompletion){0};
    }
}
#endif
