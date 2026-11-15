/** @file
 * @brief Own address-sensitive resources with atomic lifecycle accounting.
 * Audit counters are synchronized; every payload/resource belongs to one context.
 */
/** Native resource with no Ama handle or context pointer. */
typedef struct { void *resource; void *address; unsigned value; } IsolatedPayload;
static LifecycleBarrier constructor_barrier, destructor_barrier;
static _Atomic size_t initialized, finalized, copied;

/** Allocate one independent credit at the final stable payload address. */
static AmaStatus payload_initialize(void *storage) {
    IsolatedPayload *payload = storage;
    *payload = (IsolatedPayload){.address = storage, .resource = malloc(1)};
    if (payload->resource == NULL) { return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0); }
    atomic_fetch_add(&initialized, 1);
    return ama_status_success();
}

/** Both factories reach their own constant constructor before either returns. */
static AmaStatus payload_literal(void *storage, AmaByteView bytes) {
    assert(bytes.length == 1 && bytes.data[0] == 37);
    barrier_meet(&constructor_barrier);
    AmaStatus status = payload_initialize(storage);
    if (status.code == AMA_OK) { ((IsolatedPayload *)storage)->value = 37; }
    return status;
}

/** Clone into a distinct allocation without mutating source resource metadata. */
static AmaStatus payload_clone(void *storage, const void *source) {
    const IsolatedPayload *original = source;
    assert(original->address == source && original->resource != NULL);
    AmaStatus status = payload_initialize(storage);
    if (status.code == AMA_OK) {
        ((IsolatedPayload *)storage)->value = original->value;
        atomic_fetch_add(&copied, 1);
    }
    return status;
}

/** Constant teardown overlaps across contexts; consume each credit exactly once. */
static AmaStatus payload_destroy(void *storage) {
    IsolatedPayload *payload = storage;
    assert(payload->address == storage && payload->resource != NULL);
    if (payload->value == 37) { barrier_meet(&destructor_barrier); }
    free(payload->resource);
    *payload = (IsolatedPayload){0};
    atomic_fetch_add(&finalized, 1);
    return ama_status_success();
}
