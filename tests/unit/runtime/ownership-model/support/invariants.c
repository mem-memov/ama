/** @file
 * @brief Compare exact owned identities, native values/resources, service authority and stale generations after every model transition.
 */
/** Verify the canonical self/peer graph and native data of one live scalar tree while collecting its three independent native resources. */
static void check_model_tree(AmaHandle handle, const AmaHandle children[2], const size_t numbers[3], void **resources) {
    RootValue *root = object_payload(handle);
    assert(ama_handle_equal(root->left, children[0]) && ama_handle_equal(root->right, children[1]));
    assert(ama_handle_equal(root->selected, root->left) && ama_handle_equal(root->self, handle));
    const NativeValue *native[] = {&root->native,
        &((const LeafValue *)object_payload(root->left))->native,
        &((const LeafValue *)object_payload(root->right))->native};
    for (size_t index = 0; index < 3; index++) {
        assert(native[index]->resource != NULL && native[index]->address == native[index] && native[index]->number == numbers[index]);
        resources[index] = native[index]->resource;
    }
    for (size_t index = 0; index < 2; index++) {
        const LeafValue *child = object_payload(children[index]);
        assert(ama_handle_equal(child->peer, children[index]) && ama_handle_is_empty(child->root));
        assert(ama_handle_equal(ownership_cell(children[index])->owner, handle));
    }
}
/** Compare every observable API state to the independent model; no private reservation may remain after an operation returns. */
static void assert_ownership_model(const OwnershipModel *model) {
    assert(!world.failed && !world.operation_active && world.construction_work == NULL);
    assert(ama_handle_equal(program.current, model->caller));
    const RootValue *caller = object_payload(model->caller);
    const AmaServiceState *service = &ownership_cell(model->caller)->metadata.service;
    assert(service->activity == AMA_INSTANCE_RUNNING && service->return_link.attached && service->return_link.host);
    assert(ama_handle_equal(caller->self, model->caller_self) && ama_handle_equal(caller->selected, model->caller_selected));
    assert(service->last_refusal.code == (model->refusal ? AMA_BUSY : AMA_OK));
    if (model->held == 2) { assert(ama_handle_is_empty(service->outgoing)); }
    else {
        const OwnershipSlot *slot = &model->slots[model->held];
        assert(slot->live);
        AmaHandle target = model->child ? slot->children[0] : slot->handle;
        assert(ama_handle_equal(service->outgoing, target));
        assert(service->outgoing_previous == (!model->child && slot->resume ? AMA_INSTANCE_YIELDED : AMA_INSTANCE_READY));
    }
    void *resources[9] = {caller->native.resource,
        ((const LeafValue *)object_payload(caller->left))->native.resource,
        ((const LeafValue *)object_payload(caller->right))->native.resource};
    size_t resource_count = 3, roots = 1;
    assert(caller->native.number == 3 && caller->native.address == &caller->native);
    assert(caller->native.resource != NULL);
    assert(ama_handle_equal(caller->left, model->caller_children[0]) && ama_handle_equal(caller->right, model->caller_children[1]));
    for (size_t index = 0; index < 2; index++) {
        const LeafValue *child = object_payload(model->caller_children[index]);
        assert(child->native.number == (index == 0 ? 42 : 2) && child->native.address == &child->native && child->native.resource != NULL);
        assert(ama_handle_equal(child->peer, model->caller_children[index]));
        assert(ama_handle_equal(ownership_cell(model->caller_children[index])->owner, model->caller));
    }

    for (size_t index = 0; index < 2; index++) {
        const OwnershipSlot *slot = &model->slots[index];
        if (!slot->live) { continue; }
        roots++;
        assert(ownership_cell(slot->handle) == slot->cell && ama_handle_is_empty(slot->cell->owner));
        check_model_tree(slot->handle, slot->children, slot->numbers, resources + resource_count);
        resource_count += 3;
        const AmaServiceState *root = &slot->cell->metadata.service;
        AmaInstanceActivity expected = model->held == index && !model->child ? AMA_INSTANCE_ACQUIRED
            : slot->resume ? AMA_INSTANCE_YIELDED : AMA_INSTANCE_READY;
        assert(root->activity == expected && root->resume.valid == slot->resume);
        assert(root->resume.selector == (slot->resume ? 7 : 0) && !root->return_link.attached);
        assert(root->last_refusal.code == AMA_OK && root->current_operation == 0);
        for (size_t child = 0; child < 2; child++) {
            const AmaServiceState *state = &ownership_cell(slot->children[child])->metadata.service;
            assert(state->activity == (model->held == index && model->child && child == 0 ? AMA_INSTANCE_ACQUIRED : AMA_INSTANCE_READY));
            assert(!state->resume.valid && !state->return_link.attached);
        }
    }
    for (size_t left = 0; left < resource_count; left++) {
        for (size_t right = left + 1; right < resource_count; right++) { assert(resources[left] != resources[right]); }
    }
    assert(root_type.pool.live_count == roots && leaf_type.pool.live_count == roots * 2);
    size_t occupied = 0;
    for (size_t type = 0; type < 2; type++) {
        const AmaPool *pool = &types[type]->pool;
        for (size_t block = 0; block < pool->block_count; block++) { occupied += pool->blocks[block].occupied; }
    }
    assert(occupied == roots * 3);
    for (size_t index = 0; index < model->retired_count; index++) {
        AmaHandle old = model->retired[index];
        AmaPoolCell *cell = NULL;
        AmaStatus status = ama_pool_lookup(&ama_world_type(&world, old.type_id)->pool, old, true, &cell);
        assert(status.code == AMA_STALE_REFERENCE && cell == NULL);
    }
}
