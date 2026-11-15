/** @file
 * @brief Run real native workers and concurrent completion/close races without an Ama executor.
 * [ASYNC-05](../../../../../spec/language/async-and-parallelism.md#async-05)
 * gives workers immutable tokens, native byte owners and callback leases only.
 */
#include "../common.c"
#include <pthread.h>
/** Native-only worker record. Parent scope and thread join own its lifetime explicitly. */
typedef struct {
    AmaNativeEventSubmission submission;
    pthread_t thread;
    unsigned code;
    bool pause;
    _Atomic bool prepared, proceed;
    AmaNativeEventPostResult posted;
} Worker;
/** Copy native stack bytes, optionally pause for close, then post and acknowledge callback ownership. */
static void *complete_worker(void *resource) {
    Worker *worker = resource;
    unsigned char bytes[] = {(unsigned char)worker->code, 0};
    AmaNativeEventBuffer buffer = {0};
    ok(ama_native_event_buffer_copy(&worker->submission, (AmaByteView){bytes, sizeof(bytes)}, &buffer));
    atomic_store_explicit(&worker->prepared, true, memory_order_release);
    while (worker->pause && !atomic_load_explicit(&worker->proceed, memory_order_acquire)) { thrd_yield(); }
    worker->posted = ama_native_event_post(&worker->submission, worker->code, &buffer);
    ama_native_event_buffer_drop(&buffer);
    ama_native_event_submission_drop(&worker->submission);
    return NULL;
}
/** Launch retains a separate callback lease before the borrowed submit argument returns. */
static void launch(const AmaNativeEventSubmission *submission, void *resource) {
    Worker *worker = resource;
    ok(ama_native_event_submission_clone(submission, &worker->submission));
    int started = pthread_create(&worker->thread, NULL, complete_worker, worker);
    assert(started == 0);
}
/** Require acknowledgment through an actual thread join, without sleeps or timing-based ordering. */
static void join_worker(Worker *worker) {
    void *result = (void *)1;
    int joined = pthread_join(worker->thread, &result);
    assert(joined == 0 && result == NULL && worker->submission.state == NULL);
}
/** Native-only service-factory record used to race the one-time image-domain publication. */
typedef struct {
    AmaNativeEventToken token;
    _Atomic unsigned retirements;
    _Atomic bool *proceed;
} Factory;
/** Construct/dispose an independent native service while preserving its immutable scalar identity. */
static void *create_worker(void *resource) {
    Factory *factory = resource;
    while (!atomic_load_explicit(factory->proceed, memory_order_acquire)) { thrd_yield(); }
    AmaNativeEventService *owner = service(1, &factory->retirements);
    AmaNativeEventSubmission submission = prepare(owner);
    factory->token = submission.token;
    ama_native_event_service_dispose(&owner);
    ama_native_event_submission_drop(&submission);
    return NULL;
}
/** Concurrent first factories agree on one permanent domain but reserve distinct service serials. */
static void it_issues_unique_services_during_concurrent_first_domain_initialization(void) {
    enum { COUNT = 8 };
    Factory factories[COUNT] = {0};
    pthread_t threads[COUNT];
    _Atomic bool proceed = false;
    for (size_t index = 0; index < COUNT; index++) {
        factories[index].proceed = &proceed;
        int started = pthread_create(&threads[index], NULL, create_worker, &factories[index]);
        assert(started == 0);
    }
    atomic_store_explicit(&proceed, true, memory_order_release);
    for (size_t index = 0; index < COUNT; index++) {
        void *result = (void *)1;
        int joined = pthread_join(threads[index], &result);
        assert(joined == 0 && result == NULL && atomic_load(&factories[index].retirements) == 1);
        for (size_t prior = 0; prior < index; prior++) {
            assert(factories[index].token.domain == factories[prior].token.domain);
            assert(factories[index].token.service != factories[prior].token.service);
        }
    }
    assert(atomic_load(&event_test_live) == 1);
}
/** Different worker completions are retained once each with matching immutable tokens and owned bytes. */
static void it_queues_concurrent_workers_without_executing_an_ama_context(void) {
    size_t live = baseline();
    enum { COUNT = 16 };
    _Atomic unsigned retirements = 0;
    AmaNativeEventService *owner = service(COUNT, &retirements);
    AmaNativeEventSubmission submissions[COUNT];
    Worker workers[COUNT] = {0};
    for (size_t index = 0; index < COUNT; index++) {
        submissions[index] = prepare(owner);
        workers[index].code = (unsigned)index;
        ok(ama_native_event_service_arm(owner, &submissions[index], launch, &workers[index]));
    }
    for (size_t index = 0; index < COUNT; index++) {
        join_worker(&workers[index]);
        assert(workers[index].posted == AMA_EVENT_QUEUED);
    }
    bool seen[COUNT] = {false};
    for (size_t index = 0; index < COUNT; index++) {
        AmaNativeEventCompletion completion = {0};
        bool available = false;
        ok(ama_native_event_service_take(owner, &completion, &available));
        assert(available && completion.code >= 0 && completion.code < COUNT);
        size_t producer = (size_t)completion.code;
        assert(!seen[producer] && ama_native_event_token_equal(completion.token, submissions[producer].token));
        seen[producer] = true;
        assert(completion.buffer.length == 2 && completion.buffer.data[0] == producer && completion.buffer.data[1] == 0);
        ama_native_event_completion_drop(&completion);
    }
    ama_native_event_service_dispose(&owner);
    for (size_t index = 0; index < COUNT; index++) { ama_native_event_submission_drop(&submissions[index]); }
    assert(atomic_load(&retirements) == 1 && atomic_load(&event_test_live) == live);
}
/** Competing callbacks for one token commit one result; every losing producer releases its own payload. */
static void it_commits_exactly_one_of_concurrent_duplicate_callbacks(void) {
    size_t live = baseline();
    enum { COUNT = 12 };
    _Atomic unsigned retirements = 0;
    AmaNativeEventService *owner = service(1, &retirements);
    AmaNativeEventSubmission submission = prepare(owner);
    ok(ama_native_event_service_arm(owner, &submission, pending, NULL));
    Worker workers[COUNT] = {0};
    for (size_t index = 0; index < COUNT; index++) {
        workers[index].code = (unsigned)index;
        launch(&submission, &workers[index]);
    }
    size_t accepted = 0;
    for (size_t index = 0; index < COUNT; index++) {
        join_worker(&workers[index]);
        if (workers[index].posted == AMA_EVENT_QUEUED) { accepted++; }
        else { assert(workers[index].posted == AMA_EVENT_DUPLICATE); }
    }
    assert(accepted == 1);
    AmaNativeEventCompletion completion = {0};
    bool available = false;
    ok(ama_native_event_service_take(owner, &completion, &available));
    assert(available && completion.code >= 0 && completion.code < COUNT);
    assert(workers[completion.code].posted == AMA_EVENT_QUEUED && completion.buffer.data[0] == completion.code);
    ama_native_event_completion_drop(&completion);
    ok(ama_native_event_service_take(owner, &completion, &available));
    assert(!available);
    ama_native_event_service_dispose(&owner);
    ama_native_event_submission_drop(&submission);
    assert(atomic_load(&retirements) == 1 && atomic_load(&event_test_live) == live);
}
/** Closing releases cells immediately but waits for late callback/buffer acknowledgment before native retirement. */
static void it_rejects_late_completion_after_close_and_retires_only_after_acknowledgment(void) {
    size_t live = baseline();
    _Atomic unsigned retirements = 0;
    AmaNativeEventService *owner = service(1, &retirements);
    AmaNativeEventSubmission submission = prepare(owner);
    Worker worker = {.code = 7, .pause = true};
    ok(ama_native_event_service_arm(owner, &submission, launch, &worker));
    while (!atomic_load_explicit(&worker.prepared, memory_order_acquire)) { thrd_yield(); }
    ama_native_event_submission_drop(&submission);
    ama_native_event_service_dispose(&owner);
    assert(owner == NULL && atomic_load(&retirements) == 0);
    atomic_store_explicit(&worker.proceed, true, memory_order_release);
    join_worker(&worker);
    assert(worker.posted == AMA_EVENT_CLOSED && atomic_load(&retirements) == 1);
    assert(atomic_load(&event_test_live) == live);
}
/** Force a real worker to finish before submit returns; its completion is still only native queued data. */
static void joined_launch(const AmaNativeEventSubmission *submission, void *resource) {
    Worker *worker = resource;
    launch(submission, worker);
    join_worker(worker);
}
/** A native thread completing during arm cannot disappear before the owner's recheck. */
static void it_retains_worker_completion_before_the_submit_routine_returns(void) {
    size_t live = baseline();
    _Atomic unsigned retirements = 0;
    AmaNativeEventService *owner = service(1, &retirements);
    AmaNativeEventSubmission submission = prepare(owner);
    Worker worker = {.code = 9};
    ok(ama_native_event_service_arm(owner, &submission, joined_launch, &worker));
    bool ready = false;
    ok(ama_native_event_ready(&submission, &ready));
    assert(ready && worker.posted == AMA_EVENT_QUEUED);
    ama_native_event_service_dispose(&owner);
    ama_native_event_submission_drop(&submission);
    assert(atomic_load(&retirements) == 1 && atomic_load(&event_test_live) == live);
}
/** Detach an armed operation while its worker holds bytes, then reuse the cell before late publication. */
static void it_rejects_a_detached_worker_after_immediate_cell_reuse(void) {
    size_t live = baseline();
    _Atomic unsigned retirements = 0;
    AmaNativeEventService *owner = service(1, &retirements);
    AmaNativeEventSubmission old = prepare(owner);
    Worker worker = {.code = 17, .pause = true};
    ok(ama_native_event_service_arm(owner, &old, launch, &worker));
    while (!atomic_load_explicit(&worker.prepared, memory_order_acquire)) { thrd_yield(); }
    bool retired = false;
    ok(ama_native_event_service_detach(owner, old.token, &retired));
    assert(retired);
    AmaNativeEventSubmission current = prepare(owner);
    assert(current.token.slot == old.token.slot && current.token.generation != old.token.generation);
    ok(ama_native_event_service_arm(owner, &current, pending, NULL));
    atomic_store_explicit(&worker.proceed, true, memory_order_release);
    join_worker(&worker);
    assert(worker.posted == AMA_EVENT_STALE);
    bool ready = true;
    ok(ama_native_event_ready(&current, &ready));
    assert(!ready);
    assert(ama_native_event_post(&current, 23, NULL) == AMA_EVENT_QUEUED);
    AmaNativeEventCompletion completion = {0};
    bool available = false;
    ok(ama_native_event_service_take(owner, &completion, &available));
    assert(available && completion.code == 23);
    ama_native_event_completion_drop(&completion);
    ama_native_event_service_dispose(&owner);
    ama_native_event_submission_drop(&current);
    ama_native_event_submission_drop(&old);
    assert(atomic_load(&retirements) == 1 && atomic_load(&event_test_live) == live);
}
/** Run startup identity, parallel publication and ordered close/acknowledgment boundaries explicitly. */
void run(void) {
    it_issues_unique_services_during_concurrent_first_domain_initialization();
    it_queues_concurrent_workers_without_executing_an_ama_context();
    it_commits_exactly_one_of_concurrent_duplicate_callbacks();
    it_rejects_late_completion_after_close_and_retires_only_after_acknowledgment();
    it_retains_worker_completion_before_the_submit_routine_returns();
    it_rejects_a_detached_worker_after_immediate_cell_reuse();
}
/** Return after real worker threads have acknowledged every retained native resource. */
int main(void) { run(); return 0; }
