/** @file
 * @brief Start the assembled native host with explicit per-worker ownership and native-only synchronization.
 * Included first as launcher text; these test controls never supply generated Ama state.
 */
#include <assert.h>
#include <pthread.h>
#include "project.c"

/** One test executor and its independently retained native endpoint. */
typedef struct {
    AmaProjectHost host;
    AmaNativeEventSubmission endpoint;
    AmaHostResult result;
    AmaAccessMode mode;
    unsigned index, captures, received;
} ParallelWorker;

static pthread_mutex_t parallel_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t parallel_condition = PTHREAD_COND_INITIALIZER;
static unsigned first_arrivals;
static bool peer_entered, release_peer;
/** Test controller only; generated code obtains every Ama owner through its explicit view. */
static _Thread_local ParallelWorker *current_worker;

/** Keep assertion effects explicit even when compiling a release consumer. */
static void require_at(bool value, const char *expression, const char *path, int line) {
    if (!value) { fprintf(stderr, "%s:%d: Parallel host invariant failed: %s\n", path, line, expression); abort(); }
    assert(value);
}
#define require(value) require_at((value), #value, __FILE__, __LINE__)

/** Publish one actual generated entry/resume result to the joining controller. */
static void *execute_worker(void *argument) {
    ParallelWorker *worker = argument;
    current_worker = worker;
    worker->result = ama_project_execute(&worker->host, worker->mode);
    current_worker = NULL;
    return NULL;
}

/** Spawn before observing any worker-owned mutable state. */
static pthread_t start_worker(ParallelWorker *worker, AmaAccessMode mode) {
    worker->mode = mode;
    pthread_t thread;
    int result = pthread_create(&thread, NULL, execute_worker, worker);
    require(result == 0);
    return thread;
}

/** Join publishes worker results and ends every operation-scoped Ama borrow. */
static void join_worker(pthread_t thread) {
    void *value = &thread;
    int result = pthread_join(thread, &value);
    require(result == 0 && value == NULL);
}
