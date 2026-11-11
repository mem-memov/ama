#include "../common.c"

static AmaPoolCell *lookup(AmaHandle handle) {
    AmaObjectType *type = ama_world_type(&world, handle.type_id);
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_pool_lookup(&type->pool, handle, false, &cell);
    assert(status.code == AMA_OK);
    return cell;
}

static void it_treats_runtime_identity_self_assignment_as_a_noop_but_rejects_self_continuation_copy(void) {
    setup_objects();
    AmaHandle root;
    AmaStatus status = ama_object_create(&world, 2, NULL, &root);
    assert(status.code == AMA_OK);
    RootValue *value = object_payload(root);
    lookup(root)->metadata.service.activity = AMA_INSTANCE_YIELDED;
    lookup(root)->metadata.service.resume = (AmaResumePosition){true, 42};
    lookup(value->left)->metadata.service.activity = AMA_INSTANCE_WAITING;
    leaf_type.hooks.clone = NULL;
    size_t allocations = allocation_count;
    AmaHandle alias = root;
    status = ama_object_replace(&world, alias, root, false);
    assert(status.code == AMA_OK && allocation_count == allocations && cloned == 0 && destroyed == 0);
    assert(lookup(root)->metadata.service.resume.selector == 42);
    status = ama_object_replace(&world, alias, root, true);
    assert(status.code == AMA_INVALID_STATE && allocation_count == allocations && cloned == 0 && destroyed == 0);
    lookup(value->left)->metadata.service.activity = AMA_INSTANCE_READY;
    status = ama_object_destroy(&world, root);
    assert(status.code == AMA_OK);
    destroy_objects();
}

static void it_rejects_ancestor_descendant_copy_in_both_directions_before_native_hooks(void) {
    setup_objects();
    AmaFieldRecipe fields[6] = {0};
    fields[2].count = 2;
    AmaRecipe recipe = {.fields = fields, .field_count = 6};
    AmaHandle root;
    AmaStatus status = ama_object_create(&world, 2, &recipe, &root);
    assert(status.code == AMA_OK);
    RootValue *value = object_payload(root);
    for (size_t continuation = 0; continuation < 2; continuation++) {
        status = ama_object_replace(&world, root, value->branches.items[0], continuation != 0);
        assert(status.code == AMA_INVALID_STATE && cloned == 0 && destroyed == 0);
        status = ama_object_replace(&world, value->branches.items[0], root, continuation != 0);
        assert(status.code == AMA_INVALID_STATE && cloned == 0 && destroyed == 0);
    }
    lookup(root)->metadata.service.activity = AMA_INSTANCE_RUNNING;
    RootValue *first = object_payload(value->branches.items[0]);
    first->self = value->branches.items[1];
    status = ama_object_replace(&world, value->branches.items[1], value->branches.items[0], false);
    assert(status.code == AMA_OK && cloned == 3);
    RootValue *second = object_payload(value->branches.items[1]);
    assert(second->self.slot_id == value->branches.items[1].slot_id);
    assert(lookup(root)->metadata.service.activity == AMA_INSTANCE_RUNNING);
    lookup(root)->metadata.service.activity = AMA_INSTANCE_READY;
    status = ama_object_destroy(&world, root);
    assert(status.code == AMA_OK);
    destroy_objects();
}

static void it_preflights_every_source_and_destination_node_before_hooks_or_destruction(void) {
    for (size_t side = 0; side < 2; side++) {
        for (size_t node = 0; node < 9; node++) {
            for (int activity = AMA_INSTANCE_RUNNING; activity <= AMA_INSTANCE_ACQUIRED; activity++) {
                if (activity == AMA_INSTANCE_YIELDED) { continue; }
                setup_objects();
                AmaFieldRecipe fields[6] = {0};
                fields[2].count = 2;
                AmaRecipe recipe = {.fields = fields, .field_count = 6};
                AmaHandle source, destination;
                AmaStatus status = ama_object_create(&world, 2, &recipe, &source);
                assert(status.code == AMA_OK);
                status = ama_object_create(&world, 2, &recipe, &destination);
                assert(status.code == AMA_OK);
                AmaHandle selected = side == 0 ? source : destination;
                RootValue *root = object_payload(selected);
                RootValue *first = object_payload(root->branches.items[0]);
                RootValue *second = object_payload(root->branches.items[1]);
                AmaHandle nodes[] = {selected, root->left, root->right, root->branches.items[0],
                    first->left, first->right, root->branches.items[1], second->left, second->right};
                lookup(nodes[node])->metadata.service.activity = (AmaInstanceActivity)activity;
                status = ama_object_replace(&world, destination, source, false);
                assert(status.code == AMA_BUSY && cloned == 0 && destroyed == 0);
                status = ama_object_destroy(&world, selected);
                assert(status.code == AMA_BUSY && destroyed == 0);
                assert(lookup(nodes[node])->metadata.service.activity == (AmaInstanceActivity)activity);
                lookup(nodes[node])->metadata.service.activity = AMA_INSTANCE_READY;
                status = ama_object_destroy(&world, destination);
                assert(status.code == AMA_OK);
                status = ama_object_destroy(&world, source);
                assert(status.code == AMA_OK);
                destroy_objects();
            }
        }
    }
}

static void it_finalizes_a_failed_nested_replacement_and_active_links_without_emergency_allocation(void) {
    setup_objects();
    AmaHandle parent, source;
    AmaFieldRecipe fields[6] = {0};
    fields[2].count = 1;
    AmaRecipe recipe = {.fields = fields, .field_count = 6};
    AmaStatus status = ama_object_create(&world, 2, &recipe, &parent);
    assert(status.code == AMA_OK);
    status = ama_object_create(&world, 2, NULL, &source);
    assert(status.code == AMA_OK);
    RootValue *root = object_payload(parent);
    AmaHandle destination = root->branches.items[0];
    failed_destructor = 1;
    status = ama_object_replace(&world, destination, source, false);
    assert(status.code == AMA_IO_FAILURE && world.failed && destroyed == 6);
    lookup(parent)->metadata.service.activity = AMA_INSTANCE_WAITING;
    lookup(root->left)->metadata.service.activity = AMA_INSTANCE_ACQUIRED;
    size_t allocations = allocation_count;
    failure_at = allocation_count + 1;
    status = ama_object_destroy(&world, parent);
    assert(status.code == AMA_OK && allocation_count == allocations && destroyed == 9);
    status = ama_object_destroy(&world, source);
    assert(status.code == AMA_OK && allocation_count == allocations && constructed == destroyed);
    destroy_objects();
}

static void it_leaves_a_delete_unchanged_on_every_preflight_allocation_failure(void) {
    setup_objects();
    AmaHandle root;
    AmaStatus status = ama_object_create(&world, 2, NULL, &root);
    assert(status.code == AMA_OK);
    size_t start = allocation_count;
    status = ama_object_destroy(&world, root);
    assert(status.code == AMA_OK);
    size_t count = allocation_count - start;
    destroy_objects();
    for (size_t failure = 1; failure <= count; failure++) {
        setup_objects();
        status = ama_object_create(&world, 2, NULL, &root);
        assert(status.code == AMA_OK);
        failure_at = allocation_count + failure;
        status = ama_object_destroy(&world, root);
        assert(status.code == AMA_OUT_OF_MEMORY && destroyed == 0);
        assert(lookup(root)->metadata.service.activity == AMA_INSTANCE_READY);
        status = ama_object_destroy(&world, root);
        assert(status.code == AMA_OK);
        destroy_objects();
    }
}

static void it_refuses_native_reentry_while_the_complete_source_tree_is_protected(void) {
    setup_objects();
    AmaHandle source, copy;
    AmaStatus status = ama_object_create(&world, 2, NULL, &source);
    assert(status.code == AMA_OK);
    protected_source = source;
    status = ama_object_clone(&world, source, false, &copy);
    assert(status.code == AMA_OK && reentry_attempts == 3);
    protected_source = (AmaHandle){0};
    status = ama_object_destroy(&world, copy);
    assert(status.code == AMA_OK);
    status = ama_object_destroy(&world, source);
    assert(status.code == AMA_OK);
    destroy_objects();
}

static void it_reuses_its_own_authorized_source_protection_across_nested_initializer_boundaries(void) {
    setup_objects();
    AmaHandle source, result;
    AmaFieldRecipe source_fields[6] = {0};
    source_fields[2].count = 1;
    AmaRecipe source_recipe = {.fields = source_fields, .field_count = 6};
    AmaStatus status = ama_object_create(&world, 2, &source_recipe, &source);
    assert(status.code == AMA_OK);
    RootValue *root = object_payload(source);
    AmaRecipe children[2] = {
        {.constructor = AMA_CONSTRUCT_CLONE, .source = root->branches.items[0]},
        {.constructor = AMA_CONSTRUCT_CLONE, .source = source}
    };
    AmaFieldRecipe fields[6] = {0};
    fields[2] = (AmaFieldRecipe){.count = 2, .children = children};
    AmaRecipe recipe = {.fields = fields, .field_count = 6};
    status = ama_object_create(&world, 2, &recipe, &result);
    assert(status.code == AMA_OK && cloned == 9);
    status = ama_object_destroy(&world, result);
    assert(status.code == AMA_OK);
    status = ama_object_destroy(&world, source);
    assert(status.code == AMA_OK);
    destroy_objects();
}

static void it_checks_owned_list_element_capability_even_when_the_list_is_empty(void) {
    setup_objects();
    AmaObjectType list_type;
    const AmaObjectField field = {AMA_FIELD_OWNING_LIST, 1, 0};
    AmaStatus status = ama_object_type_init(&list_type, sources.allocator, 3, sizeof(AmaHandleList), 2, 3,
        &field, 1, sizeof(AmaHandleList), (AmaPayloadHooks){0});
    assert(status.code == AMA_OK);
    AmaObjectType *registry[] = {&leaf_type, &root_type, &list_type};
    world.types = registry;
    world.type_count = 3;
    AmaHandle source, copy = {0};
    status = ama_object_create(&world, 3, NULL, &source);
    assert(status.code == AMA_OK && constructed == 0);
    leaf_type.hooks.clone = NULL;
    status = ama_object_clone(&world, source, false, &copy);
    assert(status.code == AMA_INVALID_STATE && copy.generation == 0 && cloned == 0);
    status = ama_object_destroy(&world, source);
    assert(status.code == AMA_OK);
    ama_pool_dispose(&list_type.pool);
    destroy_objects();
}

static void it_validates_empty_stale_and_wrong_type_copy_sources_before_capability_checks(void) {
    setup_objects();
    AmaHandle source, copy = {0};
    AmaStatus status = ama_object_create(&world, 2, NULL, &source);
    assert(status.code == AMA_OK);
    status = ama_object_clone(&world, (AmaHandle){0}, false, &copy);
    assert(status.code == AMA_EMPTY_REFERENCE && copy.generation == 0);
    status = ama_object_clone(&world, (AmaHandle){99, 0, 1}, false, &copy);
    assert(status.code == AMA_TYPE_MISMATCH && copy.generation == 0);
    status = ama_object_destroy(&world, source);
    assert(status.code == AMA_OK);
    root_type.hooks.clone = NULL;
    status = ama_object_clone(&world, source, false, &copy);
    assert(status.code == AMA_STALE_REFERENCE && copy.generation == 0);
    destroy_objects();
}

void run(void) {
    it_treats_runtime_identity_self_assignment_as_a_noop_but_rejects_self_continuation_copy();
    it_rejects_ancestor_descendant_copy_in_both_directions_before_native_hooks();
    it_preflights_every_source_and_destination_node_before_hooks_or_destruction();
    it_finalizes_a_failed_nested_replacement_and_active_links_without_emergency_allocation();
    it_leaves_a_delete_unchanged_on_every_preflight_allocation_failure();
    it_refuses_native_reentry_while_the_complete_source_tree_is_protected();
    it_reuses_its_own_authorized_source_protection_across_nested_initializer_boundaries();
    it_checks_owned_list_element_capability_even_when_the_list_is_empty();
    it_validates_empty_stale_and_wrong_type_copy_sources_before_capability_checks();
}

int main(void) {
    run();
    return EXIT_SUCCESS;
}
