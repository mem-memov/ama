/** @file
 * @brief Hold an actual native borrow and retain native-only pending work across executor handoff.
 * Neither callback resources nor native payloads retain any Ama view or handle.
 */
/** Called from the actual native entry; a condition barrier exposes its live interval. */
void test_transfer_visit(size_t calls) {
    int result = pthread_mutex_lock(&controller.mutex);
    require(result == 0 && calls == controller.visits + 1 && controller.visits < 3);
    controller.visiting_thread[controller.visits++] = executor_index;
    if (calls == 1) {
        controller.inside_native = true;
        result = pthread_cond_broadcast(&controller.condition);
        require(result == 0);
        while (!controller.release_native) {
            result = pthread_cond_wait(&controller.condition, &controller.mutex);
            require(result == 0);
        }
    }
    result = pthread_mutex_unlock(&controller.mutex);
    require(result == 0);
}

#ifndef TEST_AFFINE
/** Submission does no work until the native host explicitly publishes completion. */
static void transfer_pending(const AmaNativeEventSubmission *submission, void *unused) {
    (void)submission;
    (void)unused;
}

/** A synchronous visitor may retain only independently owned native endpoint rights. */
AmaStatus test_transfer_service(const ama_t_0a13545c236dc688542e3824b23bf3848f38fdb81d6b4c289732f8a82eaa888f *source,
    void *unused) {
    (void)unused;
    AmaStatus status = ama_native_event_service_prepare(source->payload->service, &controller.endpoint);
    if (status.code == AMA_OK) {
        status = ama_native_event_service_arm(source->payload->service, &controller.endpoint, transfer_pending, NULL);
    }
    return status;
}
#endif
