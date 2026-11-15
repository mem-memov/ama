/** @file
 * @brief Prove retained native wake credits across idle checks, interruptions and racing posts.
 * [ASYNC-07](../../../../../spec/language/async-and-parallelism.md#async-07)
 * requires atomic accounting for queued or concurrently arriving notifications before blocking.
 */
#include "../common.c"
#include <errno.h>
#include <pthread.h>

/** Actual POSIX functions remain responsible for wake behavior; wrappers only observe/inject boundaries. */
int __real_sem_init(sem_t *, int, unsigned);
/** Retire the actual initialized semaphore. */
int __real_sem_destroy(sem_t *);
/** Publish an actual OS credit. */
int __real_sem_post(sem_t *);
/** Consume an actual OS credit, potentially blocking. */
int __real_sem_wait(sem_t *);
/** Exact initialized-resource/notification observations and one-shot failure controls. */
static _Atomic size_t live_wakes, posts, waits;
/** Inject resource creation failure before any native identity is published. */
static bool fail_initialize;
/** Inject one wait errno or hold exactly the pre-kernel-wait boundary. */
static _Atomic int wait_error;
/** Gate the pre-wait gap without depending on elapsed time or scheduler timing. */
static _Atomic bool pause_wait, release_wait;

/** Count successful real initialization and preserve failure ownership. */
int __wrap_sem_init(sem_t *wake, int shared, unsigned value) {
    assert(shared == 0 && value == 0);
    if (fail_initialize) { errno = ENOMEM; return -1; }
    int result = __real_sem_init(wake, shared, value);
    if (result == 0) { atomic_fetch_add(&live_wakes, 1); }
    return result;
}
/** Count final retirement after all callback credits have acknowledged. */
int __wrap_sem_destroy(sem_t *wake) {
    int result = __real_sem_destroy(wake);
    assert(result == 0);
    size_t prior = atomic_fetch_sub(&live_wakes, 1);
    assert(prior != 0);
    return result;
}
/** Observe posts while retaining the real semaphore's behavior. */
int __wrap_sem_post(sem_t *wake) {
    atomic_fetch_add(&posts, 1);
    return __real_sem_post(wake);
}
/** Publish a precise interleaving boundary before the real potentially blocking operation. */
int __wrap_sem_wait(sem_t *wake) {
    atomic_fetch_add(&waits, 1);
    int error = atomic_exchange(&wait_error, 0);
    if (error != 0) { errno = error; return -1; }
    while (atomic_load(&pause_wait) && !atomic_load(&release_wait)) { thrd_yield(); }
    return __real_sem_wait(wake);
}

/** Callback job owns only a native endpoint; test coordination never contains Ama state. */
typedef struct {
    AmaNativeEventSubmission endpoint;
    size_t after_wait;
    AmaNativeEventPostResult result;
} PostJob;
/** Post only after the selected native wait boundary has actually been reached. */
static void *post_after_boundary(void *resource) {
    PostJob *job = resource;
    while (atomic_load(&waits) < job->after_wait) { thrd_yield(); }
    job->result = ama_native_event_post(&job->endpoint, 17, NULL);
    atomic_store(&release_wait, true);
    ama_native_event_submission_drop(&job->endpoint);
    return NULL;
}
/** Authenticate the retained result, consume it and retire its independent byte credit. */
static void consume(AmaNativeEventService *owner, AmaNativeEventToken token) {
    AmaNativeEventCompletion completion = {0};
    bool available = false;
    ok(ama_native_event_service_take(owner, &completion, &available));
    assert(available && ama_native_event_token_equal(token, completion.token));
    ama_native_event_completion_drop(&completion);
}
/** Check the exact armed partition after all producers in this test have joined. */
static void partition(AmaNativeEventService *owner) {
    size_t armed = 0;
    for (size_t index = 0; index < owner->capacity; index++) {
        if (owner->cells[index].state == AMA_EVENT_ARMED) { armed++; }
    }
    assert(armed == owner->armed);
}

/** Factory failure preserves keepalive, leaves no resource and consumes no service serial. */
static void it_unwinds_wake_initialization_before_publishing_identity(void) {
    assert(atomic_load(&event_test_live) == 0 && atomic_load(&live_wakes) == 0);
    _Atomic unsigned retirements = 0;
    AmaNativeEventKeepalive keepalive = {&retirements, retired};
    AmaNativeEventService *owner = NULL;
    fail_initialize = true;
    AmaStatus status = ama_native_event_service_create(1, &keepalive, &owner);
    fail_initialize = false;
    assert(status.code == AMA_NATIVE_FAILURE && owner == NULL && keepalive.resource == &retirements);
    assert(atomic_load(&event_test_live) == 0 && atomic_load(&live_wakes) == 0);
    keepalive.release(keepalive.resource);
    owner = service(1, &retirements);
    AmaNativeEventSubmission endpoint = prepare(owner);
    assert(endpoint.token.service == 1);
    ama_native_event_service_dispose(&owner);
    assert(atomic_load(&live_wakes) == 1 && atomic_load(&retirements) == 1);
    ama_native_event_submission_drop(&endpoint);
    assert(atomic_load(&live_wakes) == 0 && atomic_load(&retirements) == 2);
}
/** Free/prepared-only owners return idle immediately; closed resources preserve failed outputs. */
static void it_reports_native_idleness_without_waiting_or_reviving_closed_owners(void) {
    size_t live = baseline(), before = atomic_load(&waits);
    _Atomic unsigned retirements = 0;
    AmaNativeEventService *owner = service(1, &retirements);
    bool ready = true;
    ok(ama_native_event_service_wait(owner, &ready));
    assert(!ready && atomic_load(&waits) == before);
    AmaNativeEventReadiness state = {true, true};
    ok(ama_native_event_service_poll(owner, &state));
    assert(!state.ready && !state.pending);
    AmaNativeEventSubmission endpoint = prepare(owner);
    ok(ama_native_event_service_wait(owner, &ready));
    assert(!ready && atomic_load(&waits) == before);
    ok(ama_native_event_service_poll(owner, &state));
    assert(!state.ready && !state.pending);
    ok(ama_native_event_service_arm(owner, &endpoint, pending, NULL));
    ok(ama_native_event_service_poll(owner, &state));
    assert(!state.ready && state.pending);
    assert(ama_native_event_post(&endpoint, 17, NULL) == AMA_EVENT_QUEUED);
    ok(ama_native_event_service_poll(owner, &state));
    assert(state.ready && !state.pending);
    consume(owner, endpoint.token);
    AmaNativeEventKeepalive lease = {0};
    ok(ama_native_event_service_lease(owner, &lease));
    ama_native_event_service_dispose(&owner);
    ready = true;
    AmaStatus status = ama_native_event_service_wait(lease.resource, &ready);
    assert(status.code == AMA_INVALID_STATE && ready && atomic_load(&waits) == before);
    state = (AmaNativeEventReadiness){true, true};
    status = ama_native_event_service_poll(lease.resource, &state);
    assert(status.code == AMA_INVALID_STATE && state.ready && state.pending);
    status = ama_native_event_service_poll(NULL, &state);
    assert(status.code == AMA_INVALID_STATE && state.ready && state.pending);
    status = ama_native_event_service_wait(NULL, &ready);
    assert(status.code == AMA_INVALID_STATE && ready);
    ama_native_event_submission_drop(&endpoint);
    lease.release(lease.resource);
    assert(atomic_load(&live_wakes) == 0 && atomic_load(&event_test_live) == live);
}
/** A retained credit closes the exact gap between empty metadata check and actual kernel wait. */
static void it_retains_a_post_in_the_check_to_sleep_gap_and_retries_interruptions(void) {
    size_t live = baseline();
    for (unsigned interruption = 0; interruption < 2; interruption++) {
        _Atomic unsigned retirements = 0;
        AmaNativeEventService *owner = service(1, &retirements);
        AmaNativeEventSubmission endpoint = prepare(owner);
        ok(ama_native_event_service_arm(owner, &endpoint, pending, NULL));
        PostJob job = {.after_wait = atomic_load(&waits) + 1 + interruption};
        ok(ama_native_event_submission_clone(&endpoint, &job.endpoint));
        atomic_store(&pause_wait, true);
        atomic_store(&release_wait, false);
        atomic_store(&wait_error, interruption ? EINTR : 0);
        pthread_t producer;
        int created = pthread_create(&producer, NULL, post_after_boundary, &job);
        assert(created == 0);
        bool ready = false;
        ok(ama_native_event_service_wait(owner, &ready));
        int joined = pthread_join(producer, NULL);
        assert(joined == 0 && ready && job.result == AMA_EVENT_QUEUED);
        atomic_store(&pause_wait, false);
        partition(owner);
        consume(owner, endpoint.token);
        ama_native_event_service_dispose(&owner);
        ama_native_event_submission_drop(&endpoint);
        assert(atomic_load(&live_wakes) == 0 && atomic_load(&event_test_live) == live);
    }
}
/** Polling many accepted results reserves one credit; a later blocking wait drains at most that one stale credit. */
static void it_coalesces_polled_notifications_and_drains_one_stale_credit(void) {
    size_t live = baseline(), before = atomic_load(&posts);
    _Atomic unsigned retirements = 0;
    AmaNativeEventService *owner = service(1, &retirements);
    for (unsigned cycle = 0; cycle < 8; cycle++) {
        AmaNativeEventSubmission endpoint = prepare(owner);
        ok(ama_native_event_service_arm(owner, &endpoint, pending, NULL));
        assert(ama_native_event_post(&endpoint, 17, NULL) == AMA_EVENT_QUEUED);
        bool ready = false;
        ok(ama_native_event_service_wait(owner, &ready));
        assert(ready);
        consume(owner, endpoint.token);
        ama_native_event_submission_drop(&endpoint);
        partition(owner);
    }
    assert(atomic_load(&posts) == before + 1);
    int credits = 0;
    int observed = sem_getvalue(&owner->wake, &credits);
    assert(observed == 0 && credits == 1);
    AmaNativeEventSubmission endpoint = prepare(owner);
    ok(ama_native_event_service_arm(owner, &endpoint, pending, NULL));
    size_t prior_waits = atomic_load(&waits);
    PostJob job = {.after_wait = prior_waits + 2};
    ok(ama_native_event_submission_clone(&endpoint, &job.endpoint));
    pthread_t producer;
    int created = pthread_create(&producer, NULL, post_after_boundary, &job);
    assert(created == 0);
    bool ready = false;
    ok(ama_native_event_service_wait(owner, &ready));
    int joined = pthread_join(producer, NULL);
    assert(joined == 0 && ready && atomic_load(&waits) == prior_waits + 2);
    consume(owner, endpoint.token);
    ama_native_event_service_dispose(&owner);
    ama_native_event_submission_drop(&endpoint);
    assert(atomic_load(&live_wakes) == 0 && atomic_load(&event_test_live) == live);
}
/** A real wait error changes no producer state and preserves its output rather than pretending readiness. */
static void it_reports_wait_errors_without_consuming_the_pending_operation(void) {
    size_t live = baseline();
    _Atomic unsigned retirements = 0;
    AmaNativeEventService *owner = service(1, &retirements);
    AmaNativeEventSubmission endpoint = prepare(owner);
    ok(ama_native_event_service_arm(owner, &endpoint, pending, NULL));
    atomic_store(&wait_error, EIO);
    bool ready = true;
    AmaStatus status = ama_native_event_service_wait(owner, &ready);
    assert(status.code == AMA_IO_FAILURE && ready);
    partition(owner);
    assert(owner->armed == 1 && ama_native_event_post(&endpoint, 17, NULL) == AMA_EVENT_QUEUED);
    ok(ama_native_event_service_wait(owner, &ready));
    assert(ready);
    consume(owner, endpoint.token);
    ama_native_event_service_dispose(&owner);
    ama_native_event_submission_drop(&endpoint);
    assert(atomic_load(&live_wakes) == 0 && atomic_load(&event_test_live) == live);
}
/** Run real OS resource, exact race, finite-credit and failure contracts independently of Ama. */
void run(void) {
    it_unwinds_wake_initialization_before_publishing_identity();
    it_reports_native_idleness_without_waiting_or_reviving_closed_owners();
    it_retains_a_post_in_the_check_to_sleep_gap_and_retries_interruptions();
    it_coalesces_polled_notifications_and_drains_one_stale_credit();
    it_reports_wait_errors_without_consuming_the_pending_operation();
}
/** Exit only after all native wait resources and external producer rights have retired. */
int main(void) { run(); return 0; }
