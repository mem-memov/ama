/**
 * @file
 * @brief Verify private clone and constant rollback after malformed native completion.
 */

/** A completed root clone owns its copied descendants even when transport validation fails. */
static void it_unwinds_a_completed_private_clone_without_changing_its_source_root(void) {
    setup_protocols();
    context_types[1].hooks.clone = protocol_clone;
    uint64_t identity = create_context();
    AmaContextLease lease = {0};
    AmaContextView view = enter_context(identity, &lease);
    AmaContextReference source = create_root(&lease, view);
    AmaPoolCell *original = view.world->first_root;
    AmaHandle output = {0};
    AmaStatus status = ama_object_clone(view.world, source.local, false, &output);
    assert(status.code == AMA_INVALID_STATE && ama_handle_is_empty(output));
    assert(protocol_clones == 1 && constructed == 7 && destroyed == 3);
    assert(view.world->first_root == original && view.world->last_root == original);
    assert(original->metadata.identity.live && original->metadata.service.activity == AMA_INSTANCE_READY);
    const size_t expected[] = {4, 3, 2};
    assert(memcmp(destruction_order, expected, sizeof(expected)) == 0);
    leave_context(&lease);
    finish_protocol_failure(identity, AMA_INVALID_STATE);
    assert(destroyed == 7);
}

/** Failed constant cloning cleans its raw-success native payload and earlier shared values. */
static void it_retires_every_constant_clone_resource_before_failed_factory_return(void) {
    setup_protocols();
    context_types[0].hooks.clone = protocol_clone;
    const AmaProgramSource source = {.kind = AMA_SOURCE_SHARED, .shared = 0, .type_id = 1};
    AmaContextConstant constants[2] = {context_constant, context_constant};
    constants[1].initializers = &source;
    definition.constants = constants;
    definition.constant_count = 2;
    uint64_t identity = 0;
    AmaStatus status = ama_context_create(&registry, &definition, &identity);
    assert(status.code == AMA_INVALID_STATE && identity == 0);
    assert(protocol_clones == 1 && constructed == 2 && destroyed == 2 && live_allocations == 0);
    assert(registry.first == NULL && registry.pending == 0);
    finish_contexts();
}
