#include "../common.c"

static AmaPoolCell *cell(AmaHandle handle) {
    AmaPoolCell *result = NULL;
    AmaObjectType *type = ama_world_type(&world, handle.type_id);
    AmaStatus status = ama_pool_lookup(&type->pool, handle, false, &result);
    assert(status.code == AMA_OK);
    return result;
}

static void it_remaps_root_ancestor_forward_and_cyclic_references_but_preserves_external_targets(void) {
    setup_objects();
    AmaHandle source, external, copy;
    AmaStatus status = ama_object_create(&world, 1, NULL, &external);
    assert(status.code == AMA_OK);
    cell(external)->metadata.service.activity = AMA_INSTANCE_RUNNING;
    AmaFieldRecipe fields[6] = {0};
    fields[2].count = 2;
    fields[5].count = 4;
    AmaRecipe recipe = {.fields = fields, .field_count = 6};
    status = ama_object_create(&world, 2, &recipe, &source);
    assert(status.code == AMA_OK);
    RootValue *original = object_payload(source);
    RootValue *first_branch = object_payload(original->branches.items[0]);
    RootValue *second_branch = object_payload(original->branches.items[1]);
    original->selected = first_branch->left;
    original->self = source;
    original->references.items[0] = original->left;
    original->references.items[1] = external;
    original->references.items[2] = (AmaHandle){1, 999, 1};
    ((LeafValue *)object_payload(original->left))->peer = original->right;
    ((LeafValue *)object_payload(original->right))->peer = original->left;
    ((LeafValue *)object_payload(original->left))->root = source;
    ((LeafValue *)object_payload(first_branch->left))->peer = second_branch->right;
    status = ama_object_clone(&world, source, false, &copy);
    assert(status.code == AMA_OK && cloned == 9);
    RootValue *result = object_payload(copy);
    assert(result->native.resource != original->native.resource && result->native.number == original->native.number);
    assert(result->self.slot_id == copy.slot_id && result->self.generation == copy.generation);
    assert(result->references.items[0].slot_id == result->left.slot_id);
    assert(result->references.items[1].slot_id == external.slot_id);
    assert(result->references.items[2].slot_id == 999 && result->references.items[3].generation == 0);
    assert(((LeafValue *)object_payload(result->left))->peer.slot_id == result->right.slot_id);
    assert(((LeafValue *)object_payload(result->right))->peer.slot_id == result->left.slot_id);
    assert(((LeafValue *)object_payload(result->left))->root.slot_id == copy.slot_id);
    RootValue *new_first = object_payload(result->branches.items[0]);
    RootValue *new_second = object_payload(result->branches.items[1]);
    assert(result->selected.slot_id == new_first->left.slot_id);
    assert(((LeafValue *)object_payload(new_first->left))->peer.slot_id == new_second->right.slot_id);
    status = ama_object_destroy(&world, copy);
    assert(status.code == AMA_OK);
    status = ama_object_destroy(&world, source);
    assert(status.code == AMA_OK);
    cell(external)->metadata.service.activity = AMA_INSTANCE_READY;
    status = ama_object_destroy(&world, external);
    assert(status.code == AMA_OK);
    destroy_objects();
}

static void it_copies_saved_positions_only_in_explicit_continuation_mode(void) {
    for (size_t continuation = 0; continuation < 2; continuation++) {
        setup_objects();
        AmaHandle source, copy;
        AmaStatus status = ama_object_create(&world, 2, NULL, &source);
        assert(status.code == AMA_OK);
        RootValue *root = object_payload(source);
        cell(source)->metadata.service.activity = AMA_INSTANCE_YIELDED;
        cell(source)->metadata.service.resume = (AmaResumePosition){true, 11};
        cell(root->left)->metadata.service.activity = AMA_INSTANCE_YIELDED;
        cell(root->left)->metadata.service.resume = (AmaResumePosition){true, 22};
        status = ama_object_clone(&world, source, continuation != 0, &copy);
        assert(status.code == AMA_OK);
        RootValue *result = object_payload(copy);
        assert(cell(copy)->metadata.service.resume.selector == (continuation ? 11 : 0));
        assert(cell(result->left)->metadata.service.resume.selector == (continuation ? 22 : 0));
        assert(cell(result->right)->metadata.service.resume.selector == 0);
        assert(!cell(copy)->metadata.service.return_link.attached);
        assert(cell(source)->metadata.service.resume.selector == 11);
        status = ama_object_destroy(&world, copy);
        assert(status.code == AMA_OK);
        status = ama_object_destroy(&world, source);
        assert(status.code == AMA_OK);
        destroy_objects();
    }
}

static void it_refuses_busy_descendants_and_missing_clone_capabilities_before_native_hooks(void) {
    for (size_t scenario = 0; scenario < 2; scenario++) {
        setup_objects();
        AmaHandle source, copy = {0};
        AmaStatus status = ama_object_create(&world, 2, NULL, &source);
        assert(status.code == AMA_OK);
        RootValue *root = object_payload(source);
        if (scenario == 0) { cell(root->right)->metadata.service.activity = AMA_INSTANCE_WAITING; }
        else { leaf_type.hooks.clone = NULL; }
        status = ama_object_clone(&world, source, false, &copy);
        assert(status.code == (scenario == 0 ? AMA_BUSY : AMA_INVALID_STATE) && cloned == 0 && copy.generation == 0);
        cell(root->right)->metadata.service.activity = AMA_INSTANCE_READY;
        status = ama_object_destroy(&world, source);
        assert(status.code == AMA_OK);
        destroy_objects();
    }
}

static void it_preserves_the_source_and_releases_each_failed_clone_prefix(void) {
    size_t count = 0;
    for (size_t failure = 0; failure <= count; failure++) {
        setup_objects();
        AmaHandle source, copy = {0};
        AmaStatus status = ama_object_create(&world, 2, NULL, &source);
        assert(status.code == AMA_OK);
        RootValue *original = object_payload(source);
        void *resource = original->native.resource;
        size_t start = allocation_count;
        failure_at = failure == 0 ? 0 : start + failure;
        status = ama_object_clone(&world, source, false, &copy);
        if (failure == 0) {
            assert(status.code == AMA_OK);
            count = allocation_count - start;
            status = ama_object_destroy(&world, copy);
            assert(status.code == AMA_OK);
        } else {
            assert(status.code == AMA_OUT_OF_MEMORY && copy.generation == 0);
            assert(root_type.pool.live_count == 1 && leaf_type.pool.live_count == 2);
            assert(original->native.resource == resource && cell(source)->metadata.service.activity == AMA_INSTANCE_READY);
        }
        status = ama_object_destroy(&world, source);
        assert(status.code == AMA_OK);
        destroy_objects();
    }
}

static void it_uses_independent_clone_contexts_for_repeated_source_initializers(void) {
    setup_objects();
    AmaHandle source, root;
    AmaStatus status = ama_object_create(&world, 1, NULL, &source);
    assert(status.code == AMA_OK);
    LeafValue *original = object_payload(source);
    original->peer = source;
    cell(source)->metadata.service.activity = AMA_INSTANCE_YIELDED;
    cell(source)->metadata.service.resume = (AmaResumePosition){true, 17};
    AmaRecipe copy = {.constructor = AMA_CONSTRUCT_CLONE, .source = source, .continuation = true};
    AmaFieldRecipe fields[6] = {{.children = &copy}, {.children = &copy}};
    AmaRecipe recipe = {.fields = fields, .field_count = 6};
    status = ama_object_create(&world, 2, &recipe, &root);
    assert(status.code == AMA_OK && cloned == 2 && constructed == 4);
    RootValue *result = object_payload(root);
    LeafValue *left = object_payload(result->left);
    LeafValue *right = object_payload(result->right);
    assert(left->peer.slot_id == result->left.slot_id && right->peer.slot_id == result->right.slot_id);
    assert(left->native.resource != right->native.resource && left->native.resource != original->native.resource);
    assert(cell(result->left)->metadata.service.resume.selector == 17);
    status = ama_object_destroy(&world, root);
    assert(status.code == AMA_OK);
    status = ama_object_destroy(&world, source);
    assert(status.code == AMA_OK);
    destroy_objects();
}

void run(void) {
    it_remaps_root_ancestor_forward_and_cyclic_references_but_preserves_external_targets();
    it_copies_saved_positions_only_in_explicit_continuation_mode();
    it_refuses_busy_descendants_and_missing_clone_capabilities_before_native_hooks();
    it_preserves_the_source_and_releases_each_failed_clone_prefix();
    it_uses_independent_clone_contexts_for_repeated_source_initializers();
}

int main(void) {
    run();
    return EXIT_SUCCESS;
}
