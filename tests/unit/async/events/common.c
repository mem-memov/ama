/** @file
 * @brief Observe native allocations and code-resource retirement without storing any Ama identity.
 * [ASYNC-05](../../../../spec/language/async-and-parallelism.md#async-05)
 * separates the callback resource domain from executor-owned Ama storage.
 */
#ifndef AMA_NATIVE_EVENTS_TEST_COMMON
#define AMA_NATIVE_EVENTS_TEST_COMMON
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include "native/events/internal.h"
/** Actual libc allocation used only by the linker-wrapped failure fixture. */
void *__real_malloc(size_t);
/** Actual libc release used only by the linker-wrapped failure fixture. */
void __real_free(void *);
/** Native allocation observations; callback tests update them concurrently. */
static _Atomic size_t event_test_allocations, event_test_live, event_test_fail;
/** Inject one selected native allocation failure; no fake allocation replaces production storage. */
void *__wrap_malloc(size_t size) {
    size_t index = atomic_fetch_add(&event_test_allocations, 1) + 1;
    if (index == atomic_load(&event_test_fail)) { return NULL; }
    void *memory = __real_malloc(size);
    if (memory != NULL) { atomic_fetch_add(&event_test_live, 1); }
    return memory;
}
/** Count actual native owner retirement while leaving libc/internal thread allocations alone. */
void __wrap_free(void *memory) {
    if (memory != NULL) { size_t prior = atomic_fetch_sub(&event_test_live, 1); assert(prior != 0); }
    __real_free(memory);
}
/** Check actual native status without performing a side effect inside an assertion. */
static void ok(AmaStatus status) { assert(status.code == AMA_OK); }
/** Supply a native-only resource retirement observer, never a callback into Ama. */
static void retired(void *resource) { atomic_fetch_add((_Atomic unsigned *)resource, 1); }
/** Create a real fixed-capacity service and transfer an observed native keepalive. */
static AmaNativeEventService *service(size_t capacity, _Atomic unsigned *retirements) {
    AmaNativeEventService *owner = NULL;
    AmaNativeEventKeepalive keepalive = {retirements, retired};
    ok(ama_native_event_service_create(capacity, &keepalive, &owner));
    assert(owner != NULL && keepalive.resource == NULL && keepalive.release == NULL);
    return owner;
}
/** Reserve a real immutable token/lease with the actual native cell allocator. */
static AmaNativeEventSubmission prepare(AmaNativeEventService *owner) {
    AmaNativeEventSubmission submission = {0};
    ok(ama_native_event_service_prepare(owner, &submission));
    return submission;
}
/** Keep a submitted operation pending until the test's actual callback posts its result. */
static void pending(const AmaNativeEventSubmission *submission, void *resource) {
    (void)submission; (void)resource;
}
/** Reserve the one legitimate process-lifetime image receipt before counted test lifetimes. */
static size_t baseline(void) {
    _Atomic unsigned retirements = 0;
    AmaNativeEventService *owner = service(1, &retirements);
    ama_native_event_service_dispose(&owner);
    assert(owner == NULL && atomic_load(&retirements) == 1);
    return atomic_load(&event_test_live);
}
#endif
