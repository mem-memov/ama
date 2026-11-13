/**
 * @file
 * @brief Verify cleanup credit for default, literal and shared-source native construction.
 */

/** Malformed zero-code default completion still owns its native resource and children. */
static void it_cleans_a_completed_default_payload_after_malformed_success(void) {
    setup_protocols();
    context_types[1].hooks.initialize = protocol_initialize;
    uint64_t identity = create_context();
    AmaContextLease lease = {0};
    AmaContextView view = enter_context(identity, &lease);
    AmaHandle output = {99, 88, 77};
    AmaStatus status = ama_object_create(view.world, 2, NULL, &output);
    assert(status.code == AMA_INVALID_STATE && output.type_id == 99 && output.generation == 77);
    assert(protocol_defaults == 1 && constructed == 4 && destroyed == 3);
    const size_t expected[] = {4, 3, 2};
    assert(memcmp(destruction_order, expected, sizeof(expected)) == 0);
    assert(view.world->first_root == NULL && view.world->last_root == NULL);
    leave_context(&lease);
    finish_protocol_failure(identity, AMA_INVALID_STATE);
    assert(destroyed == 4);
}

/** A raw-success child owns cleanup even before its parent can credit that child. */
static void it_unwinds_a_raw_success_child_before_its_parent_receives_completion_credit(void) {
    for (size_t cloning = 0; cloning < 2; cloning++) {
        setup_protocols();
        if (cloning != 0) { context_types[0].hooks.clone = protocol_clone; }
        else { context_types[0].hooks.initialize = protocol_initialize; }
        uint64_t identity = create_context();
        AmaContextLease lease = {0};
        AmaContextView view = enter_context(identity, &lease);
        AmaHandle output = {0};
        AmaStatus status;
        if (cloning != 0) {
            AmaContextReference source = create_root(&lease, view);
            status = ama_object_clone(view.world, source.local, false, &output);
            assert(constructed == 5 && protocol_clones == 1 && view.world->first_root != NULL);
        } else {
            status = ama_object_create(view.world, 2, NULL, &output);
            assert(constructed == 2 && protocol_defaults == 1 && view.world->first_root == NULL);
        }
        assert(status.code == AMA_INVALID_STATE && ama_handle_is_empty(output) && destroyed == 1);
        assert(destruction_order[0] == 2);
        leave_context(&lease);
        finish_protocol_failure(identity, AMA_INVALID_STATE);
        assert(destroyed == (cloning != 0 ? 5 : 2));
    }
}

/** A selected literal constructs once and receives cleanup after malformed completion. */
static void it_cleans_literal_construction_without_running_the_default_callback(void) {
    setup_protocols();
    definition.constant_count = 0;
    context_types[0].hooks.initialize = protocol_initialize;
    context_types[0].hooks.literal = protocol_literal;
    uint64_t identity = create_context();
    AmaContextLease lease = {0};
    AmaContextView view = enter_context(identity, &lease);
    const AmaRecipe recipe = {.constructor = AMA_CONSTRUCT_LITERAL, .literal = {context_literal_bytes, 1}};
    AmaHandle output = {0};
    AmaStatus status = ama_object_create(view.world, 1, &recipe, &output);
    assert(status.code == AMA_INVALID_STATE && ama_handle_is_empty(output));
    assert(protocol_literals == 1 && protocol_defaults == 0 && constructed == 1 && destroyed == 1);
    assert(destruction_order[0] == 37);
    leave_context(&lease);
    finish_protocol_failure(identity, AMA_INVALID_STATE);
}

/** Shared-to-owned native cloning must credit the new payload before rejecting its status. */
static void it_cleans_native_source_clone_and_preserves_the_shared_original(void) {
    setup_protocols();
    context_types[0].hooks.clone = protocol_clone;
    uint64_t identity = create_context();
    AmaContextLease lease = {0};
    AmaContextView view = enter_context(identity, &lease);
    const NativeValue *source = view.program->constants[0].storage->items->payload;
    const AmaRecipe recipe = {.constructor = AMA_CONSTRUCT_CLONE, .native_source = source};
    AmaHandle output = {0};
    AmaStatus status = ama_object_create(view.world, 1, &recipe, &output);
    assert(status.code == AMA_INVALID_STATE && ama_handle_is_empty(output));
    assert(protocol_clones == 1 && constructed == 2 && destroyed == 1);
    assert(source->resource != NULL && source->number == 37);
    leave_context(&lease);
    finish_protocol_failure(identity, AMA_INVALID_STATE);
    assert(destroyed == 2);
}

/** A nonzero constructor completion must self-release and never receive a destructor credit. */
static void it_never_destroys_a_failed_native_constructor_with_malformed_transport(void) {
    setup_protocols();
    protocol_status = ama_status_make(AMA_OUT_OF_MEMORY, NULL, 1);
    context_types[1].hooks.initialize = protocol_initialize;
    uint64_t identity = create_context();
    AmaContextLease lease = {0};
    AmaContextView view = enter_context(identity, &lease);
    AmaHandle output = {0};
    AmaStatus status = ama_object_create(view.world, 2, NULL, &output);
    assert(status.code == AMA_INVALID_STATE && ama_handle_is_empty(output));
    assert(protocol_self_releases == 1 && constructed == 4 && destroyed == 2);
    const size_t expected[] = {3, 2};
    assert(memcmp(destruction_order, expected, sizeof(expected)) == 0);
    leave_context(&lease);
    finish_protocol_failure(identity, AMA_INVALID_STATE);
    assert(destroyed == 3);
}
