/** @file
 * @brief Retain native completion rights through a synchronous typed reference visitor.
 * Store no view, Ama handle or context pointer in a native payload or callback.
 */
/** No asynchronous work is needed until the host explicitly publishes its native event. */
static void native_pending(const AmaNativeEventSubmission *submission, void *resource) {
    (void)submission;
    (void)resource;
}

/** Native bridge called only by its worker under the generated borrow scope.
 * Initial entries rendezvous; the second peer entry waits for an explicit release,
 * then fails technically after another context has imported its own completion.
 */
AmaStatus test_parallel_service(const ama_t_0a13545c236dc688542e3824b23bf3848f38fdb81d6b4c289732f8a82eaa888f *source,
    void *unused) {
    (void)unused;
    ParallelWorker *worker = current_worker;
    require(worker != NULL && worker->endpoint.state == NULL);
    AmaStatus status = ama_native_event_service_prepare(source->payload->service, &worker->endpoint);
    if (status.code != AMA_OK) { return status; }
    status = ama_native_event_service_arm(source->payload->service, &worker->endpoint, native_pending, NULL);
    if (status.code != AMA_OK) { return status; }
    worker->captures++;
    int result = pthread_mutex_lock(&parallel_mutex);
    require(result == 0);
    if (worker->captures == 1) {
        first_arrivals++;
        result = pthread_cond_broadcast(&parallel_condition);
        require(result == 0);
        while (first_arrivals != 2) {
            result = pthread_cond_wait(&parallel_condition, &parallel_mutex);
            require(result == 0);
        }
    } else if (worker->index == 1) {
        peer_entered = true;
        result = pthread_cond_broadcast(&parallel_condition);
        require(result == 0);
        while (!release_peer) {
            result = pthread_cond_wait(&parallel_condition, &parallel_mutex);
            require(result == 0);
        }
        status = ama_status_make(AMA_NATIVE_FAILURE, "Isolated parallel failure",
            sizeof("Isolated parallel failure") - 1);
    }
    result = pthread_mutex_unlock(&parallel_mutex);
    require(result == 0);
    return status;
}

/** An ordinary Ama branch reaches this only after its own packet was imported. */
void test_parallel_received(void) {
    require(current_worker != NULL);
    current_worker->received++;
}

/** Queue owned binary bytes using only native rights, never by entering a context. */
static void post_completion(AmaNativeEventSubmission *endpoint, AmaNativeEventPostResult expected) {
    const unsigned char bytes[] = {0, 255, 66};
    AmaNativeEventBuffer buffer = {0};
    AmaStatus status = ama_native_event_buffer_copy(endpoint, (AmaByteView){bytes, sizeof(bytes)}, &buffer);
    if (expected == AMA_EVENT_CLOSED) {
        require(status.code == AMA_INVALID_STATE && buffer.owner == NULL && buffer.data == NULL);
    } else { require(status.code == AMA_OK); }
    AmaNativeEventPostResult posted = ama_native_event_post(endpoint, 7, &buffer);
    require(posted == expected);
    if (expected == AMA_EVENT_QUEUED) { require(buffer.owner == NULL); }
    ama_native_event_buffer_drop(&buffer);
}
