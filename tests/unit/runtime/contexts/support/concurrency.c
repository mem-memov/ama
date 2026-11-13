/**
 * @file
 * @brief Run independent guarded state updates and concurrent empty-context factories.
 *
 * Thread joins synchronize observations. Only the system allocator runs concurrently; counted
 * native fixture hooks are never called from the state-update worker.
 */
#ifndef AMA_CONTEXT_CONCURRENCY_TEST_C
#define AMA_CONTEXT_CONCURRENCY_TEST_C
/** Concurrent empty factories use no instrumented non-thread-safe fixture allocator/hooks. */
typedef struct {
    AmaContextRegistry *registry;
    uint64_t identities[32];
} ContextFactoryThread;

/** Publish independent empty contexts through the actual shared identity issuer and registry. */
static int context_factory_worker(void *argument) {
    ContextFactoryThread *input = argument;
    const AmaContextDefinition empty = {0};
    for (size_t index = 0; index < 32; index++) {
        AmaStatus status = ama_context_create(input->registry, &empty, &input->identities[index]);
        assert(status.code == AMA_OK && input->identities[index] != 0);
    }
    return 0;
}

/** Thread input contains only copyable checked identities and an externally synchronized observation. */
typedef struct {
    uint64_t busy;
    uint64_t independent;
    AmaContextReference reference;
    bool completed;
} ContextThread;

/** A concurrent caller refuses the held context but acquires independent state without native allocation. */
static int context_worker(void *argument) {
    ContextThread *input = argument;
    AmaContextLease lease = {0};
    AmaStatus status = ama_context_enter(&registry, input->busy, &lease);
    assert(status.code == AMA_BUSY && lease.context == NULL);
    (void)enter_context(input->independent, &lease);
    root_value(&lease, input->reference)->native.number = 900;
    leave_context(&lease);
    input->completed = true;
    return 0;
}


#endif
