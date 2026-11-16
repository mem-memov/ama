/** @file
 * @brief Issue service identities across disposal and physical image reloads.
 * [ASYNC-05](../../../../spec/language/async-and-parallelism.md#async-05)
 * requires new services not to revive old submission tokens. One unrecycled
 * native receipt per loaded image keeps allocator/address reuse from doing so.
 */
#include <stdlib.h>
#include <threads.h>
#include "native/events/internal.h"
/** The winning receipt is process-lifetime native identity history, with no code/release dependency. Component contract: [ASYNC-05](../../../../spec/language/async-and-parallelism.md#async-05). */
static _Atomic(void *) event_image_domain;
/** Serialize only the fixed-width service sequence; receipt allocation happens outside this gate. Component contract: [ASYNC-05](../../../../spec/language/async-and-parallelism.md#async-05). */
static _Atomic bool event_identity_locked;
/** Never reset or wrap while this physical image is resident. Component contract: [ASYNC-05](../../../../spec/language/async-and-parallelism.md#async-05). */
static uint64_t event_identity_sequence;

AmaStatus ama_native_event_identity(AmaNativeEventToken *identity) {
    void *domain = atomic_load_explicit(&event_image_domain, memory_order_acquire);
    if (domain == NULL) {
        void *candidate = malloc(1);
        if (candidate == NULL) { return AMA_NATIVE_EVENT_ERROR(AMA_OUT_OF_MEMORY, "Cannot reserve native event domain"); }
        void *expected = NULL;
        if (atomic_compare_exchange_strong_explicit(&event_image_domain, &expected, candidate,
                memory_order_acq_rel, memory_order_acquire)) {
            domain = candidate;
        } else {
            domain = expected;
            free(candidate);
        }
    }
    while (atomic_exchange_explicit(&event_identity_locked, true, memory_order_acquire)) { thrd_yield(); }
    uint64_t next;
    bool reserved = ama_native_event_sequence(event_identity_sequence, UINT64_MAX, &next);
    if (reserved) { event_identity_sequence = next; }
    atomic_store_explicit(&event_identity_locked, false, memory_order_release);
    if (!reserved) { return AMA_NATIVE_EVENT_ERROR(AMA_LIMIT_EXCEEDED, "Native service identities exhausted"); }
    *identity = (AmaNativeEventToken){.domain = domain, .service = next, .slot = AMA_NATIVE_EVENT_NONE};
    return ama_status_success();
}
