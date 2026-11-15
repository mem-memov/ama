/** @file
 * @brief Force two native lifecycle intervals to overlap without timing assumptions.
 * Initialize and destroy these native-only barriers outside worker lifetime.
 */
/** One-shot native synchronization containing no Ama address or execution capability. */
typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    unsigned arrived;
} LifecycleBarrier;

/** Initialize before publishing the barrier to either worker. */
static void barrier_initialize(LifecycleBarrier *barrier) {
    *barrier = (LifecycleBarrier){0};
    int result = pthread_mutex_init(&barrier->mutex, NULL);
    assert(result == 0);
    result = pthread_cond_init(&barrier->condition, NULL);
    assert(result == 0);
}

/** Both participants must arrive; condition rechecks handle spurious wakeups. */
static void barrier_meet(LifecycleBarrier *barrier) {
    int result = pthread_mutex_lock(&barrier->mutex);
    assert(result == 0 && barrier->arrived < 2);
    barrier->arrived++;
    result = pthread_cond_broadcast(&barrier->condition);
    assert(result == 0);
    while (barrier->arrived != 2) {
        result = pthread_cond_wait(&barrier->condition, &barrier->mutex);
        assert(result == 0);
    }
    result = pthread_mutex_unlock(&barrier->mutex);
    assert(result == 0);
}

/** Join both workers before ending native synchronization lifetime. */
static void barrier_destroy(LifecycleBarrier *barrier) {
    assert(barrier->arrived == 2);
    int result = pthread_cond_destroy(&barrier->condition);
    assert(result == 0);
    result = pthread_mutex_destroy(&barrier->mutex);
    assert(result == 0);
}
