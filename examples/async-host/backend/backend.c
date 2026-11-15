/** @file
 * @brief Keep callback work in a separately resident native image until its host joins and acknowledges every owner.
 * [ASYNC-05](../../../spec/language/async-and-parallelism.md#async-05)
 * forbids retained Ama borrows; [EMBED-06](../../../spec/implementation/c-library.md#embed-06)
 * requires the creator image to remain pinned until these callbacks have returned.
 */
#include "backend-native.h"
#include <assert.h>
#include <pthread.h>
#include <stdlib.h>

/** The host serializes submission/finish; workers touch only their own immutable native rights and condition. */
typedef struct BackendJob {
    struct BackendJob *next;
    AmaNativeEventSubmission endpoint;
    AmaNativeEventBuffer bytes;
    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    bool proceed, threaded;
    AmaNativeEventPostResult result;
} BackendJob;
/** Native host ownership, not an Ama instance's hidden state; it survives component context disposal. */
static BackendJob *current, *last;
/** Plain retired-domain data has no callable or Ama dependency and can survive creator-image unload. */
static const void *previous_domain;
static bool changed_domain;

/** Publish native-only immutable bytes; acknowledgment may then consume its independent native rights. */
static void publish(BackendJob *job) {
    assert(job->bytes.length == 3 && job->bytes.data[0] == 0 && job->bytes.data[1] == 255 && job->bytes.data[2] == 66);
    job->result = ama_native_event_post(&job->endpoint, 0, &job->bytes);
}
/** Wait on a native condition, without retaining any C activation belonging to an Ama entry. */
static void *worker(void *resource) {
    BackendJob *job = resource;
    int result = pthread_mutex_lock(&job->mutex);
    assert(result == 0);
    while (!job->proceed) { result = pthread_cond_wait(&job->condition, &job->mutex); assert(result == 0); }
    result = pthread_mutex_unlock(&job->mutex);
    assert(result == 0);
    publish(job);
    ama_native_event_buffer_drop(&job->bytes);
    ama_native_event_submission_drop(&job->endpoint);
    return NULL;
}
/** Undo a private candidate after native initialization failure, before publication to the host. */
static AmaStatus reject(BackendJob *job, bool mutex, bool condition, AmaStatus status) {
    if (condition) { int result = pthread_cond_destroy(&job->condition); assert(result == 0); }
    if (mutex) { int result = pthread_mutex_destroy(&job->mutex); assert(result == 0); }
    ama_native_event_buffer_drop(&job->bytes);
    ama_native_event_submission_drop(&job->endpoint);
    free(job);
    return status;
}
/** Retain callback/byte rights before returning from submit; every private failure releases its prefix. */
AmaStatus backend_submit(const AmaNativeEventSubmission *endpoint, bool immediate) {
    BackendJob *job = calloc(1, sizeof(*job));
    if (job == NULL) { return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0); }
    AmaStatus status = ama_native_event_submission_clone(endpoint, &job->endpoint);
    if (status.code != AMA_OK) { return reject(job, false, false, status); }
    const unsigned char bytes[] = {0, 255, 66};
    status = ama_native_event_buffer_copy(&job->endpoint, (AmaByteView){bytes, sizeof(bytes)}, &job->bytes);
    if (status.code != AMA_OK) { return reject(job, false, false, status); }
    if (immediate) { publish(job); }
    else {
        int result = pthread_mutex_init(&job->mutex, NULL);
        if (result != 0) { return reject(job, false, false, ama_status_make(AMA_IO_FAILURE, NULL, 0)); }
        result = pthread_cond_init(&job->condition, NULL);
        if (result != 0) { return reject(job, true, false, ama_status_make(AMA_IO_FAILURE, NULL, 0)); }
        result = pthread_create(&job->thread, NULL, worker, job);
        if (result != 0) { return reject(job, true, true, ama_status_make(AMA_IO_FAILURE, NULL, 0)); }
        job->threaded = true;
    }
    changed_domain = previous_domain != NULL && previous_domain != endpoint->token.domain;
    previous_domain = endpoint->token.domain;
    if (last == NULL) { current = job; }
    else { last->next = job; }
    last = job;
    return ama_status_success();
}
/** Join before consuming endpoint rights, so the loader barrier excludes all creator-image calls before unload. */
BackendResult backend_finish(void) {
    BackendJob *job = current;
    if (job == NULL) { return BACKEND_NONE; }
    if (job->threaded) {
        int result = pthread_mutex_lock(&job->mutex);
        assert(result == 0);
        job->proceed = true;
        result = pthread_cond_signal(&job->condition);
        assert(result == 0);
        result = pthread_mutex_unlock(&job->mutex);
        assert(result == 0);
        result = pthread_join(job->thread, NULL);
        assert(result == 0);
        result = pthread_cond_destroy(&job->condition);
        assert(result == 0);
        result = pthread_mutex_destroy(&job->mutex);
        assert(result == 0);
    }
    BackendResult outcome = job->result == AMA_EVENT_QUEUED ? BACKEND_ACCEPTED
        : job->result == AMA_EVENT_STALE ? BACKEND_OBSOLETE
        : job->result == AMA_EVENT_CLOSED ? BACKEND_CLOSED : BACKEND_OTHER;
    ama_native_event_buffer_drop(&job->bytes);
    ama_native_event_submission_drop(&job->endpoint);
    assert(job->bytes.owner == NULL && job->endpoint.state == NULL);
    current = job->next;
    if (current == NULL) { last = NULL; }
    free(job);
    return outcome;
}

/** This compares immutable token data only, never an address inside an unloaded image. */
bool backend_domain_changed(void) { return changed_domain; }
