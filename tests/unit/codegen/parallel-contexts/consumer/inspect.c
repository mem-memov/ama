/** @file
 * @brief Inspect two quiescent generated hosts only through their independently acquired leases.
 * Additional explicit scalar roots demonstrate pool reuse without touching saved Ama positions.
 */
/** Hold both origin leases while comparing actual shared storage and rejecting equal foreign identities. */
static void inspect_private_storage(ParallelWorker workers[2]) {
    require(workers[0].host.identity != workers[1].host.identity);
    require(ama_handle_equal(workers[0].host.root, workers[1].host.root));
    AmaContextLease leases[2] = {0};
    AmaContextView views[2] = {0};
    for (size_t index = 0; index < 2; index++) {
        AmaStatus status = ama_context_enter(&workers[index].host.registry,
            workers[index].host.identity, &leases[index]);
        require(status.code == AMA_OK);
        status = ama_context_inspect(&leases[index], &views[index]);
        require(status.code == AMA_OK && views[index].program->constant_count > 0);
        AmaProgram *program = views[index].program;
        uint64_t scalar_type = program->constants[0].type_id;
        AmaHandle first = {0}, reused = {0};
        status = ama_object_create(views[index].world, scalar_type, NULL, &first);
        require(status.code == AMA_OK);
        status = ama_object_destroy(views[index].world, first);
        require(status.code == AMA_OK);
        status = ama_object_create(views[index].world, scalar_type, NULL, &reused);
        require(status.code == AMA_OK && first.slot_id == reused.slot_id && first.generation != reused.generation);
        AmaObjectType *type = NULL;
        AmaPoolCell *cell = NULL;
        status = ama_context_resolve(&leases[index], (AmaContextReference){workers[index].host.identity, first},
            AMA_ACCESS_READ, &type, &cell);
        require(status.code == AMA_STALE_REFERENCE && type == NULL && cell == NULL);
        status = ama_object_destroy(views[index].world, reused);
        require(status.code == AMA_OK);
    }
    require(views[0].world != views[1].world && views[0].program != views[1].program);
    require(views[0].control != views[1].control);
    require(views[0].world->types[0] != views[1].world->types[0]);
    require(views[0].program->constant_count == views[1].program->constant_count);
    for (size_t index = 0; index < views[0].program->constant_count; index++) {
        const AmaConstantStorage *left = views[0].program->constants[index].storage;
        const AmaConstantStorage *right = views[1].program->constants[index].storage;
        require(left != right && left->count == right->count);
        for (size_t item = 0; item < left->count; item++) {
            require(&left->items[item] != &right->items[item]);
            require(left->items[item].payload != right->items[item].payload);
        }
    }
    AmaObjectType *type = NULL;
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_context_resolve(&leases[1],
        (AmaContextReference){workers[0].host.identity, workers[0].host.root}, AMA_ACCESS_READ, &type, &cell);
    require(status.code == AMA_TYPE_MISMATCH && type == NULL && cell == NULL);
    for (size_t index = 0; index < 2; index++) {
        status = ama_context_leave(&leases[index]);
        require(status.code == AMA_OK);
    }
}
