/** @file
 * @brief Transfer a yielded context twice or keep its declared thread-affine native resource pinned.
 * Publish all state through mutex/condition handshakes; stop both workers before ending host lifetime.
 */
/** Execute explicit quiescence, borrow exclusion, native policy and saved-position checks. */
int main(void) {
    controller = (TransferController){0};
#ifdef TEST_AFFINE
    controller.pinned = true;
#endif
    int result = pthread_mutex_init(&controller.mutex, NULL);
    require(result == 0);
    result = pthread_cond_init(&controller.condition, NULL);
    require(result == 0);
    const unsigned indices[2] = {0, 1};
    pthread_t threads[2];
    for (size_t index = 0; index < 2; index++) {
        result = pthread_create(&threads[index], NULL, executor_worker, (void *)&indices[index]);
        require(result == 0);
    }
    submit_command(COMMAND_ENTRY);
    result = pthread_mutex_lock(&controller.mutex);
    require(result == 0);
    while (!controller.inside_native) {
        result = pthread_cond_wait(&controller.condition, &controller.mutex);
        require(result == 0);
    }
    result = pthread_mutex_unlock(&controller.mutex);
    require(result == 0);
    require(transfer_owner(1) == TRANSFER_BUSY && controller.owner == 0);
    AmaHostResult refused = ama_project_execute(&controller.host, AMA_ACCESS_RESUME);
    require(refused.status.code == AMA_BUSY);
    require(ama_project_dispose(&controller.host).code == AMA_BUSY);
    result = pthread_mutex_lock(&controller.mutex);
    require(result == 0);
    controller.release_native = true;
    result = pthread_cond_broadcast(&controller.condition);
    require(result == 0);
    result = pthread_mutex_unlock(&controller.mutex);
    require(result == 0);
    finish_command();
    require(controller.result.outcome == AMA_HOST_YIELDED && controller.visits == 1);
    AmaHandle root = controller.host.root;
    uint64_t identity = controller.host.identity;
    AmaContextLease lease = {0};
    uint64_t first_position = saved_position(&lease);
#ifdef TEST_AFFINE
    AmaStatus status = ama_context_leave(&lease);
    require(status.code == AMA_OK);
    require(transfer_owner(1) == TRANSFER_PINNED && controller.owner == 0);
#else
    require(transfer_owner(1) == TRANSFER_BUSY && controller.owner == 0);
    AmaStatus status = ama_context_leave(&lease);
    require(status.code == AMA_OK);
    require(transfer_owner(1) == TRANSFER_OK && controller.owner == 1);
    uint64_t unchanged_position = saved_position(&lease);
    require(unchanged_position == first_position);
    status = ama_context_leave(&lease);
    require(status.code == AMA_OK);
    AmaNativeEventPostResult posted = ama_native_event_post(&controller.endpoint, 7, NULL);
    require(posted == AMA_EVENT_QUEUED);
#endif
    require(controller.host.identity == identity && ama_handle_equal(controller.host.root, root));
    submit_command(COMMAND_RESUME);
    finish_command();
    require(controller.result.status.code == AMA_OK && controller.visits == 2);
#ifdef TEST_AFFINE
    require(controller.result.outcome == AMA_HOST_COMPLETED);
    require(controller.visiting_thread[0] == 0 && controller.visiting_thread[1] == 0);
    (void)first_position;
#else
    require(controller.result.outcome == AMA_HOST_YIELDED);
    require(controller.visiting_thread[0] == 0 && controller.visiting_thread[1] == 1);
    uint64_t second_position = saved_position(&lease);
    require(second_position != first_position);
    status = ama_context_leave(&lease);
    require(status.code == AMA_OK);
    require(transfer_owner(0) == TRANSFER_OK && controller.owner == 0);
    require(controller.host.identity == identity && ama_handle_equal(controller.host.root, root));
    submit_command(COMMAND_RESUME);
    finish_command();
    require(controller.result.outcome == AMA_HOST_COMPLETED && controller.result.status.code == AMA_OK);
    require(controller.visits == 3 && controller.visiting_thread[2] == 0);
#endif
    submit_command(COMMAND_DISPOSE);
    finish_command();
    require(controller.disposal.code == AMA_OK && controller.host.disposed);
#ifndef TEST_AFFINE
    posted = ama_native_event_post(&controller.endpoint, 8, NULL);
    require(posted == AMA_EVENT_CLOSED);
    ama_native_event_submission_drop(&controller.endpoint);
#endif
    result = pthread_mutex_lock(&controller.mutex);
    require(result == 0 && !controller.active && controller.command == COMMAND_NONE);
    controller.stop = true;
    result = pthread_cond_broadcast(&controller.condition);
    require(result == 0);
    result = pthread_mutex_unlock(&controller.mutex);
    require(result == 0);
    for (size_t index = 0; index < 2; index++) {
        void *value = threads;
        result = pthread_join(threads[index], &value);
        require(result == 0 && value == NULL);
    }
    result = pthread_cond_destroy(&controller.condition);
    require(result == 0);
    result = pthread_mutex_destroy(&controller.mutex);
    require(result == 0);
    return 0;
}
