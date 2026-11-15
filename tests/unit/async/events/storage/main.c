/** @file
 * @brief Check retained completion order, allocation-free publication and native ownership failures.
 * [ASYNC-05](../../../../../spec/language/async-and-parallelism.md#async-05)
 * gives completion data independent ownership; [ASYNC-06](../../../../../spec/language/async-and-parallelism.md#async-06)
 * requires synchronous completion to remain observable.
 */
#include "../common.c"
#include "runtime/library/code.h"
/** Consume the actual independent code pin when the service's final native owner retires. */
static void release_code(void *resource) {
    bool released = ama_library_code_release(resource);
    assert(released);
}
/** Synchronous native input for a submit function; the callback copies every byte before returning. */
typedef struct { int64_t code; AmaByteView bytes; } Immediate;
/** Publish directly from submission, proving the service does not hold its metadata gate across foreign C. */
static void immediate(const AmaNativeEventSubmission *submission, void *resource) {
    const Immediate *input = resource;
    AmaNativeEventBuffer buffer = {0};
    ok(ama_native_event_buffer_copy(submission, input->bytes, &buffer));
    size_t allocations = atomic_load(&event_test_allocations);
    AmaNativeEventPostResult posted = ama_native_event_post(submission, input->code, &buffer);
    assert(posted == AMA_EVENT_QUEUED && buffer.owner == NULL);
    assert(atomic_load(&event_test_allocations) == allocations);
}
/** Observed acceptance order determines FIFO independently of reservation order; every result owns copied bytes. */
static void it_retains_synchronous_completions_and_drains_in_acceptance_order(void) {
    size_t live = baseline();
    _Atomic unsigned retirements = 0;
    AmaNativeEventService *owner = service(3, &retirements);
    AmaNativeEventSubmission jobs[3];
    for (size_t index = 0; index < 3; index++) { jobs[index] = prepare(owner); }
    const size_t order[] = {2, 0, 1};
    unsigned char source[] = {'x', 0, 255};
    for (size_t index = 0; index < 3; index++) {
        Immediate input = {(int64_t)order[index], {source, sizeof(source)}};
        ok(ama_native_event_service_arm(owner, &jobs[order[index]], immediate, &input));
        bool ready = false;
        ok(ama_native_event_ready(&jobs[order[index]], &ready));
        assert(ready);
    }
    source[0] = 'z';
    AmaNativeEventSubmission full = {0};
    AmaStatus status = ama_native_event_service_prepare(owner, &full);
    assert(status.code == AMA_BUSY && full.state == NULL && owner->generation == 3);
    for (size_t index = 0; index < 3; index++) {
        AmaNativeEventCompletion completion = {0};
        bool available = false;
        ok(ama_native_event_service_take(owner, &completion, &available));
        assert(available && completion.code == (int64_t)order[index]);
        assert(ama_native_event_token_equal(completion.token, jobs[order[index]].token));
        const unsigned char expected[] = {'x', 0, 255};
        assert(completion.buffer.length == sizeof(expected) && memcmp(completion.buffer.data, expected, sizeof(expected)) == 0);
        ama_native_event_completion_drop(&completion);
    }
    AmaNativeEventCompletion empty = {0};
    bool available = true;
    ok(ama_native_event_service_take(owner, &empty, &available));
    assert(!available);
    ama_native_event_service_dispose(&owner);
    for (size_t index = 0; index < 3; index++) { ama_native_event_submission_drop(&jobs[index]); }
    assert(atomic_load(&retirements) == 1 && atomic_load(&event_test_live) == live);
}
/** No allocator is called to reject or queue a callback; rejected byte owners stay with their producer. */
static void it_preserves_payloads_on_unarmed_duplicate_and_foreign_rejection(void) {
    size_t live = baseline();
    _Atomic unsigned retirements = 0;
    AmaNativeEventService *owner = service(1, &retirements);
    AmaNativeEventService *other = service(1, &retirements);
    AmaNativeEventSubmission job = prepare(owner), foreign = prepare(other);
    AmaNativeEventBuffer buffer = {0};
    const unsigned char bytes[] = "result";
    ok(ama_native_event_buffer_copy(&job, (AmaByteView){bytes, sizeof(bytes) - 1}, &buffer));
    void *allocation = buffer.owner;
    AmaNativeEventPostResult posted = ama_native_event_post(&job, 0, &buffer);
    assert(posted == AMA_EVENT_UNARMED && buffer.owner == allocation);
    ok(ama_native_event_service_arm(owner, &job, pending, NULL));
    ok(ama_native_event_service_arm(other, &foreign, pending, NULL));
    posted = ama_native_event_post(&foreign, 0, &buffer);
    assert(posted == AMA_EVENT_INVALID_BUFFER && buffer.owner == allocation);
    posted = ama_native_event_post(&job, 0, &buffer);
    assert(posted == AMA_EVENT_QUEUED && buffer.owner == NULL);
    ok(ama_native_event_buffer_copy(&job, (AmaByteView){bytes, sizeof(bytes) - 1}, &buffer));
    allocation = buffer.owner;
    size_t allocations = atomic_load(&event_test_allocations);
    posted = ama_native_event_post(&job, 1, &buffer);
    assert(posted == AMA_EVENT_DUPLICATE && buffer.owner == allocation);
    assert(atomic_load(&event_test_allocations) == allocations);
    ama_native_event_buffer_drop(&buffer);
    ama_native_event_service_dispose(&owner);
    ama_native_event_service_dispose(&other);
    ama_native_event_submission_drop(&job);
    ama_native_event_submission_drop(&foreign);
    assert(atomic_load(&retirements) == 2 && atomic_load(&event_test_live) == live);
}
/** Service/copy failures publish no owner and leave the optional resource lease with its caller. */
static void it_unwinds_allocations_and_preserves_keepalive_on_factory_failure(void) {
    size_t live = baseline();
    for (size_t failure = 1; failure <= 2; failure++) {
        _Atomic unsigned retirements = 0;
        AmaNativeEventKeepalive keepalive = {&retirements, retired};
        AmaNativeEventService *owner = NULL;
        atomic_store(&event_test_fail, atomic_load(&event_test_allocations) + failure);
        AmaStatus status = ama_native_event_service_create(2, &keepalive, &owner);
        atomic_store(&event_test_fail, 0);
        assert(status.code == AMA_OUT_OF_MEMORY && owner == NULL && keepalive.resource == &retirements);
        assert(atomic_load(&retirements) == 0 && atomic_load(&event_test_live) == live);
        keepalive.release(keepalive.resource);
    }
    _Atomic unsigned retirements = 0;
    AmaNativeEventService *owner = service(1, &retirements);
    AmaNativeEventSubmission job = prepare(owner);
    AmaNativeEventBuffer buffer = {0};
    atomic_store(&event_test_fail, atomic_load(&event_test_allocations) + 1);
    AmaStatus status = ama_native_event_buffer_copy(&job, (AmaByteView){(const unsigned char *)"x", 1}, &buffer);
    atomic_store(&event_test_fail, 0);
    assert(status.code == AMA_OUT_OF_MEMORY && buffer.owner == NULL);
    ok(ama_native_event_service_arm(owner, &job, pending, NULL));
    size_t allocations = atomic_load(&event_test_allocations);
    atomic_store(&event_test_fail, allocations + 1);
    AmaNativeEventPostResult posted = ama_native_event_post(&job, -12, NULL);
    atomic_store(&event_test_fail, 0);
    assert(posted == AMA_EVENT_QUEUED && atomic_load(&event_test_allocations) == allocations);
    AmaNativeEventCompletion completion = {0};
    bool available = false;
    ok(ama_native_event_service_take(owner, &completion, &available));
    assert(available && completion.code == -12 && completion.buffer.owner == NULL);
    ama_native_event_completion_drop(&completion);
    ama_native_event_service_dispose(&owner);
    ama_native_event_submission_drop(&job);
    assert(atomic_load(&retirements) == 1 && atomic_load(&event_test_live) == live);
}
/** Native byte ownership retains code/resources after service/context retirement. */
static void it_keeps_consumed_bytes_alive_after_service_disposal(void) {
    size_t live = baseline();
    _Atomic unsigned retirements = 0;
    AmaNativeEventService *owner = service(1, &retirements);
    AmaNativeEventSubmission job = prepare(owner);
    Immediate input = {7, {(const unsigned char *)"retained", 8}};
    ok(ama_native_event_service_arm(owner, &job, immediate, &input));
    AmaNativeEventCompletion completion = {0};
    bool available = false;
    ok(ama_native_event_service_take(owner, &completion, &available));
    assert(available);
    ama_native_event_service_dispose(&owner);
    ama_native_event_submission_drop(&job);
    assert(atomic_load(&retirements) == 0);
    assert(completion.buffer.length == 8 && memcmp(completion.buffer.data, "retained", 8) == 0);
    ama_native_event_completion_drop(&completion);
    assert(atomic_load(&retirements) == 1 && atomic_load(&event_test_live) == live);
}
/** A real resident-code registry stays busy after owner/context release until every native lease retires. */
static void it_retains_actual_code_pins_through_callback_and_byte_owners(void) {
    size_t live = baseline();
    AmaLibraryCodePins pins = {0};
    bool retained = ama_library_code_retain(&pins);
    assert(retained);
    AmaNativeEventKeepalive code = {&pins, release_code};
    AmaNativeEventService *owner = NULL;
    ok(ama_native_event_service_create(1, &code, &owner));
    AmaNativeEventSubmission submission = prepare(owner);
    AmaNativeEventBuffer buffer = {0};
    ok(ama_native_event_buffer_copy(&submission, (AmaByteView){(const unsigned char *)"pinned", 6}, &buffer));
    ama_native_event_service_dispose(&owner);
    bool quiescent = ama_library_code_quiescent(&pins);
    assert(!quiescent && code.resource == NULL);
    ama_native_event_submission_drop(&submission);
    quiescent = ama_library_code_quiescent(&pins);
    assert(!quiescent);
    ama_native_event_buffer_drop(&buffer);
    quiescent = ama_library_code_quiescent(&pins);
    assert(quiescent && atomic_load(&event_test_live) == live);
}
/** Invalid shapes, capacity and lifecycle modes leave usable reservations intact. */
static void it_rejects_invalid_shapes_and_discards_only_unarmed_reservations(void) {
    size_t live = baseline();
    AmaNativeEventService *invalid = NULL;
    AmaStatus status = ama_native_event_service_create(0, NULL, &invalid);
    assert(status.code == AMA_INVALID_STATE && invalid == NULL);
    status = ama_native_event_service_create(SIZE_MAX, NULL, &invalid);
    assert(status.code == AMA_LIMIT_EXCEEDED && invalid == NULL);
    _Atomic unsigned retirements = 0;
    AmaNativeEventService *owner = service(1, &retirements);
    AmaNativeEventSubmission discarded = prepare(owner);
    ok(ama_native_event_service_discard(owner, discarded.token));
    bool ready = true;
    status = ama_native_event_ready(&discarded, &ready);
    assert(status.code == AMA_INVALID_STATE && ready);
    AmaNativeEventSubmission job = prepare(owner);
    ok(ama_native_event_service_arm(owner, &job, pending, NULL));
    status = ama_native_event_service_discard(owner, job.token);
    assert(status.code == AMA_INVALID_STATE);
    AmaNativeEventBuffer borrowed = {.data = (const unsigned char *)"borrowed", .length = 8};
    AmaNativeEventPostResult posted = ama_native_event_post(&job, 0, &borrowed);
    assert(posted == AMA_EVENT_INVALID_BUFFER && borrowed.data != NULL);
    AmaNativeEventBuffer buffer = {0};
    status = ama_native_event_buffer_copy(&job, (AmaByteView){NULL, 1}, &buffer);
    assert(status.code == AMA_INVALID_STATE && buffer.owner == NULL);
    status = ama_native_event_buffer_copy(&job, (AmaByteView){(const unsigned char *)"x", SIZE_MAX}, &buffer);
    assert(status.code == AMA_LIMIT_EXCEEDED && buffer.owner == NULL);
    ama_native_event_service_dispose(&owner);
    ama_native_event_submission_drop(&discarded);
    ama_native_event_submission_drop(&job);
    assert(atomic_load(&retirements) == 1 && atomic_load(&event_test_live) == live);
}
/** Exhaust callback positions relative to readiness query, Awaiting commit, return and reconciliation.
 * The abstract phase/flag variables model executor-owned protocol state; actual retained
 * records and token authentication use the production native queue in every schedule.
 */
static void it_preserves_a_completion_at_every_wait_reconciliation_boundary(void) {
    size_t live = baseline();
    for (unsigned arrival = 0; arrival < 6; arrival++) {
        _Atomic unsigned retirements = 0;
        AmaNativeEventService *owner = service(1, &retirements);
        AmaNativeEventSubmission job = prepare(owner);
        /* Publication precedes the arm call, including its synchronous callback. */
        AmaNativeEventToken published = job.token;
        bool registration_live = true, recorded_ready = false, awaiting = false;
        Immediate input = {.code = 17};
        ok(ama_native_event_service_arm(owner, &job, arrival == 0 ? immediate : pending,
            arrival == 0 ? &input : NULL));
        for (unsigned boundary = 1; boundary <= 5; boundary++) {
            if (arrival == boundary) {
                assert(registration_live && ama_native_event_token_equal(published, job.token));
                assert(ama_native_event_post(&job, 17, NULL) == AMA_EVENT_QUEUED);
            }
            if (boundary == 1) {
                bool completed = false;
                ok(ama_native_event_ready(&job, &completed));
                recorded_ready = completed;
            }
            if (boundary == 2) { awaiting = !recorded_ready; }
            if (boundary == 4 || boundary == 5) {
                AmaNativeEventCompletion completion = {0};
                bool available = false;
                ok(ama_native_event_service_take(owner, &completion, &available));
                if (available) {
                    assert(registration_live && ama_native_event_token_equal(published, completion.token));
                    assert(completion.code == 17);
                    recorded_ready = true;
                    ama_native_event_completion_drop(&completion);
                }
                if (recorded_ready) { awaiting = false; }
            }
            if (arrival < boundary || (arrival == boundary && boundary != 0)) {
                bool retained = false;
                if (!recorded_ready) { ok(ama_native_event_ready(&job, &retained)); }
                assert(recorded_ready || retained);
            }
        }
        assert(recorded_ready && !awaiting);
        assert(ama_native_event_post(&job, 18, NULL) == AMA_EVENT_STALE);
        ama_native_event_service_dispose(&owner);
        ama_native_event_submission_drop(&job);
        assert(atomic_load(&retirements) == 1 && atomic_load(&event_test_live) == live);
    }
}
/** Register synchronous, ownership, fault and preflight boundaries explicitly. */
void run(void) {
    it_preserves_a_completion_at_every_wait_reconciliation_boundary();
    it_retains_synchronous_completions_and_drains_in_acceptance_order();
    it_preserves_payloads_on_unarmed_duplicate_and_foreign_rejection();
    it_unwinds_allocations_and_preserves_keepalive_on_factory_failure();
    it_keeps_consumed_bytes_alive_after_service_disposal();
    it_retains_actual_code_pins_through_callback_and_byte_owners();
    it_rejects_invalid_shapes_and_discards_only_unarmed_reservations();
}
/** Return after all native owners have retired except the documented image receipt. */
int main(void) { run(); return 0; }
