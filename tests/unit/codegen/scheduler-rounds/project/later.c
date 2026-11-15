/** @file
 * @brief Start a native worker with independently retained endpoint ownership and no borrowed Ama state.
 * [ASYNC-05](../../../../../spec/language/async-and-parallelism.md#async-05)
 * permits posting native records only; [ASYNC-07](../../../../../spec/language/async-and-parallelism.md#async-07)
 * lets the executor wait and later import them. This standalone fixture joins all workers.
 */
#include "ama_n_abc375d0b88e7042aa7ea80d4c0fbd4b3e7458ec160eff18287406a7c03d19ed.interface.h"
#include "ama_n_abc375d0b88e7042aa7ea80d4c0fbd4b3e7458ec160eff18287406a7c03d19ed.begin.h"
#include <pthread.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <threads.h>
/** Native job survives entry return; the owning method joins before releasing its allocation. */
struct AMA_NATIVE(Job) { pthread_t thread; _Atomic bool stop; AmaNativeEventSubmission endpoint; };
/** Post immutable native identity only; no callback can dispatch Ama or access a borrowed view. */
static void *AMA_NATIVE(worker)(void *resource) {
    struct AMA_NATIVE(Job) *job = resource;
#ifdef AMA_TEST_WAIT_GATE
    while (!atomic_load(&job->stop) && !atomic_load(&ama_test_wait_entered)) { thrd_yield(); }
#endif
    if (!atomic_load(&job->stop)) { (void)ama_native_event_post(&job->endpoint, 0, NULL); }
    ama_native_event_submission_drop(&job->endpoint);
    return NULL;
}
/** Signal teardown even if a test gate was never reached; join before freeing callback storage. */
static AmaStatus AMA_NATIVE(retire)(AMA_NATIVE(State) *state) {
    if (state->job == NULL) { return ama_status_success(); }
    atomic_store(&state->job->stop, true);
    int joined = pthread_join(state->job->thread, NULL);
    if (joined != 0) { return ama_status_make(AMA_NATIVE_FAILURE, "Native worker join failed", sizeof("Native worker join failed") - 1); }
    free(state->job);
    state->job = NULL;
    return ama_status_success();
}
/** Default method owns no callback job and is unbound until its constructor supplies a native lease. */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *state) { *state = (AMA_NATIVE(State)){0}; return ama_status_success(); }
/** Retire the worker before releasing its independent method service lease. */
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) {
    AmaStatus status = AMA_NATIVE(retire)(state);
    AmaNativeEventKeepalive lease = state->lease;
    state->lease = (AmaNativeEventKeepalive){0};
    if (lease.release != NULL) { lease.release(lease.resource); }
    return status;
}
/** Capture only an owning native endpoint before pthread creation; every failure unwinds its candidate. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state, const ama_t_9db694e143579f845c79928561c197b4e40844c0c01cb856ebdf38f589594114 *parameters,
    ama_t_69ea84d2b63265c0d2d29bc6a36eb8d9f696e1210aee0bb6ceebbfc19415ced5 *results, const AmaNativeServices *services, AmaStatus *status) {
    (void)results; (void)services;
    *status = AMA_NATIVE(retire)(state);
    if (status->code != AMA_OK) { return; }
    if (state->lease.resource == NULL) { *status = ama_status_make(AMA_INVALID_STATE, NULL, 0); return; }
    struct AMA_NATIVE(Job) *job = malloc(sizeof(*job));
    if (job == NULL) { *status = ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0); return; }
    *job = (struct AMA_NATIVE(Job)){0};
    atomic_init(&job->stop, false);
    *status = ama_native_event_service_callback(state->lease.resource, parameters->AMA_INPUT(0)->payload->token, &job->endpoint);
    if (status->code != AMA_OK) { free(job); return; }
    int created = pthread_create(&job->thread, NULL, AMA_NATIVE(worker), job);
    if (created != 0) {
        ama_native_event_submission_drop(&job->endpoint);
        free(job);
        *status = ama_status_make(AMA_NATIVE_FAILURE, "Native worker creation failed", sizeof("Native worker creation failed") - 1);
        return;
    }
    state->job = job;
    *status = ama_status_success();
}
#include "ama_n_abc375d0b88e7042aa7ea80d4c0fbd4b3e7458ec160eff18287406a7c03d19ed.end.h"
