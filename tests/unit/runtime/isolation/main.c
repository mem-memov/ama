/** @file
 * @brief Audit colliding local identities and simultaneous native constant lifecycle.
 * Exercise actual runtime owners and stable pool slots without generated-code inspection.
 */
#include "common.c"

/** Native hooks overlap while every mutable Ama owner remains disjoint. */
static void it_isolates_context_storage_during_concurrent_native_constant_lifecycle(void) {
    barrier_initialize(&constructor_barrier);
    barrier_initialize(&destructor_barrier);
    atomic_store(&initialized, 0);
    atomic_store(&finalized, 0);
    atomic_store(&copied, 0);
    AmaContextRegistry registry;
    AmaStatus status = ama_context_registry_init(&registry, source_system_allocator());
    assert(status.code == AMA_OK);
    FactoryWorker workers[2] = {{.registry = &registry}, {.registry = &registry}};
    run_pair(workers, factory_worker);
    assert(workers[0].identity != workers[1].identity && atomic_load(&initialized) == 2);
    barrier_destroy(&constructor_barrier);
    AmaContextLease leases[2] = {0};
    AmaContextView views[2] = {0};
    AmaContextReference roots[2] = {0};
    const IsolatedPayload *constants[2];
    for (size_t index = 0; index < 2; index++) {
        status = ama_context_enter(&registry, workers[index].identity, &leases[index]);
        assert(status.code == AMA_OK);
        status = ama_context_inspect(&leases[index], &views[index]);
        assert(status.code == AMA_OK);
        AmaProgram *program = views[index].program;
        constants[index] = program->constants[0].storage->items->payload;
        assert(constants[index]->value == 37 && constants[index]->address == constants[index]);
        assert(views[index].world->types[0]->field_context == program);
        assert(views[index].world->types[0]->pool.live_count == 0);
        AmaHandle handle = {0};
        status = ama_object_create(views[index].world, 1, NULL, &handle);
        assert(status.code == AMA_OK);
        status = ama_context_reference(&leases[index], handle, &roots[index]);
        assert(status.code == AMA_OK);
        resolve_payload(&leases[index], roots[index])->value = 100 + (unsigned)index;
    }
    assert(ama_handle_equal(roots[0].local, roots[1].local));
    assert(constants[0] != constants[1] && constants[0]->resource != constants[1]->resource);
    assert(views[0].world != views[1].world && views[0].program != views[1].program);
    assert(views[0].control != views[1].control);
    assert(views[0].world->types[0] != views[1].world->types[0]);
    assert(views[0].program->constants[0].storage != views[1].program->constants[0].storage);
    views[0].program->failure_reported = true;
    views[0].program->current = roots[0].local;
    views[0].control->current = roots[0].local;
    assert(!views[1].program->failure_reported && ama_handle_is_empty(views[1].program->current));
    assert(ama_handle_is_empty(views[1].control->current) && views[1].status.code == AMA_OK);
    AmaObjectType *object = NULL;
    AmaPoolCell *cell = NULL;
    status = ama_context_resolve(&leases[1], roots[0], AMA_ACCESS_READ, &object, &cell);
    assert(status.code == AMA_TYPE_MISMATCH && object == NULL && cell == NULL);
    AmaHandle clone = {0};
    status = ama_object_clone(views[0].world, roots[0].local, false, &clone);
    assert(status.code == AMA_OK && atomic_load(&copied) == 1);
    status = ama_object_destroy(views[0].world, roots[0].local);
    assert(status.code == AMA_OK);
    AmaHandle reused = {0};
    status = ama_object_create(views[0].world, 1, NULL, &reused);
    assert(status.code == AMA_OK && reused.slot_id == roots[0].local.slot_id);
    assert(reused.generation != roots[0].local.generation);
    status = ama_context_resolve(&leases[0], roots[0], AMA_ACCESS_READ, &object, &cell);
    assert(status.code == AMA_STALE_REFERENCE);
    assert(resolve_payload(&leases[1], roots[1])->value == 101);
    assert(constants[1]->value == 37 && !views[1].world->failed);
    for (size_t index = 0; index < 2; index++) {
        status = ama_context_leave(&leases[index]);
        assert(status.code == AMA_OK);
    }
    run_pair(workers, disposal_worker);
    barrier_destroy(&destructor_barrier);
    assert(atomic_load(&initialized) == 6 && atomic_load(&finalized) == 6);
    assert(registry.first == NULL && registry.pending == 0);
    status = ama_context_registry_destroy(&registry);
    assert(status.code == AMA_OK);
}

/** Run the context and native lifecycle isolation audit. */
void run(void) {
    it_isolates_context_storage_during_concurrent_native_constant_lifecycle();
}

/** Successful exit requires every ownership and overlap invariant to hold. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
