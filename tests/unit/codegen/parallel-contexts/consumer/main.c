/** @file
 * @brief Drive exact overlap, native-only delivery and isolated failure against actual generated dispatchers.
 * This final consumer fragment owns all host objects until threads and endpoint credits retire.
 */
/** Execute two actual contexts and retain one healthy executor after its peer fails. */
int main(void) {
    ParallelWorker workers[2] = {{.index = 0}, {.index = 1}};
    pthread_t first = start_worker(&workers[0], AMA_ACCESS_ENTRY);
    pthread_t second = start_worker(&workers[1], AMA_ACCESS_ENTRY);
    join_worker(first);
    join_worker(second);
    require(first_arrivals == 2);
    for (size_t index = 0; index < 2; index++) {
        require(workers[index].result.outcome == AMA_HOST_YIELDED);
        require(workers[index].result.status.code == AMA_OK && workers[index].captures == 1);
        require(workers[index].host.active.world == NULL);
    }
    inspect_private_storage(workers);
    AmaNativeEventSubmission old_peer = workers[1].endpoint;
    workers[1].endpoint = (AmaNativeEventSubmission){0};
    second = start_worker(&workers[1], AMA_ACCESS_ENTRY);
    int result = pthread_mutex_lock(&parallel_mutex);
    require(result == 0);
    while (!peer_entered) {
        result = pthread_cond_wait(&parallel_condition, &parallel_mutex);
        require(result == 0);
    }
    result = pthread_mutex_unlock(&parallel_mutex);
    require(result == 0);
    AmaHostResult refused = ama_project_execute(&workers[1].host, AMA_ACCESS_RESUME);
    require(refused.status.code == AMA_BUSY);
    require(ama_project_dispose(&workers[1].host).code == AMA_BUSY);
    post_completion(&workers[0].endpoint, AMA_EVENT_QUEUED);
    post_completion(&old_peer, AMA_EVENT_CLOSED);
    first = start_worker(&workers[0], AMA_ACCESS_RESUME);
    join_worker(first);
    require(workers[0].result.outcome == AMA_HOST_COMPLETED && workers[0].received == 1);
    require(workers[0].result.status.code == AMA_OK && !workers[0].host.disposed);
    result = pthread_mutex_lock(&parallel_mutex);
    require(result == 0 && peer_entered && !release_peer);
    release_peer = true;
    result = pthread_cond_broadcast(&parallel_condition);
    require(result == 0);
    result = pthread_mutex_unlock(&parallel_mutex);
    require(result == 0);
    join_worker(second);
    require(workers[1].result.outcome == AMA_HOST_FAILED && workers[1].result.status.code == AMA_NATIVE_FAILURE);
    require(workers[1].host.disposed && !workers[1].host.registry.initialized && workers[1].received == 0);
    post_completion(&workers[1].endpoint, AMA_EVENT_CLOSED);
    ama_native_event_submission_drop(&workers[1].endpoint);
    ama_native_event_submission_drop(&old_peer);
    ama_native_event_submission_drop(&workers[0].endpoint);
    first = start_worker(&workers[0], AMA_ACCESS_ENTRY);
    join_worker(first);
    require(workers[0].result.outcome == AMA_HOST_YIELDED && workers[0].captures == 2);
    post_completion(&workers[0].endpoint, AMA_EVENT_QUEUED);
    first = start_worker(&workers[0], AMA_ACCESS_RESUME);
    join_worker(first);
    require(workers[0].result.outcome == AMA_HOST_COMPLETED && workers[0].received == 2);
    require(ama_project_dispose(&workers[0].host).code == AMA_OK);
    post_completion(&workers[0].endpoint, AMA_EVENT_CLOSED);
    ama_native_event_submission_drop(&workers[0].endpoint);
    result = pthread_cond_destroy(&parallel_condition);
    require(result == 0);
    result = pthread_mutex_destroy(&parallel_mutex);
    require(result == 0);
    return 0;
}
