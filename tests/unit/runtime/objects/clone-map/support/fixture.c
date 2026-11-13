/** @file
 * @brief Own scalar source/external trees, old colliding generations and resolver-use observations.
 */
static AmaHandle source_root, external_root, external_leaf, stale_root, stale_leaf;
static AmaHandle source_nodes[3];
static AmaServiceState source_services[3];
static size_t clone_resolutions;
/** Borrow a checked public cell only while its instance remains owned by this fixture. */
static AmaPoolCell *mapped_cell(AmaHandle handle) {
    AmaObjectType *type = ama_world_type(&world, handle.type_id);
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_pool_lookup(&type->pool, handle, false, &cell);
    assert(status.code == AMA_OK);
    return cell;
}
/** Count any attempted initializer rerun during cloning, while retaining canonical recipe behavior for a legitimate new construction. */
static AmaStatus count_recipe(AmaWorld *owner, SourceArena *arena, AmaHandle parent,
    size_t field, AmaFieldRecipe *output, void *context) {
    clone_resolutions++;
    return ama_program_field_recipe(owner, arena, parent, field, output, context);
}
/** Establish stale references sharing internal slot IDs but not generations, plus unrelated Running external targets of both nominal types. */
static void setup_map(void) {
    setup_construction_program();
    AmaStatus status = ama_program_start(&program, 2, &stale_root);
    assert(status.code == AMA_OK);
    stale_leaf = ((RootValue *)object_payload(stale_root))->left;
    status = ama_object_destroy(&world, stale_root);
    assert(status.code == AMA_OK);
    status = ama_object_create(&world, 2, NULL, &source_root);
    assert(status.code == AMA_OK);
    assert_constructed_self_tree(source_root);
    RootValue *root = object_payload(source_root);
    source_nodes[0] = source_root;
    source_nodes[1] = root->left;
    source_nodes[2] = root->right;
    assert(stale_root.slot_id == source_root.slot_id && stale_root.generation != source_root.generation);
    assert(stale_leaf.slot_id == root->left.slot_id && stale_leaf.generation != root->left.generation);
    status = ama_object_create(&world, 1, NULL, &external_leaf);
    assert(status.code == AMA_OK);
    status = ama_object_create(&world, 2, NULL, &external_root);
    assert(status.code == AMA_OK);
    mapped_cell(external_leaf)->metadata.service.activity = AMA_INSTANCE_RUNNING;
    mapped_cell(external_root)->metadata.service.activity = AMA_INSTANCE_RUNNING;
    for (size_t index = 0; index < 3; index++) {
        AmaServiceState *service = &mapped_cell(source_nodes[index])->metadata.service;
        service->activity = AMA_INSTANCE_YIELDED;
        service->resume = (AmaResumePosition){true, index + 30};
        service->current_operation = index + 40;
        service->last_refusal = ama_status_make(AMA_BUSY, "old", 3);
        source_services[index] = *service;
    }
    clone_resolutions = 0;
    leaf_type.resolve_field = root_type.resolve_field = count_recipe;
}
/** Check protected source state and both non-acquired external targets, then clean exactly the fixture roots. */
static void finish_map(void) {
    assert(clone_resolutions == 0 && !world.failed && !world.operation_active);
    for (size_t index = 0; index < 3; index++) {
        const AmaServiceState *service = &mapped_cell(source_nodes[index])->metadata.service;
        assert(service->activity == source_services[index].activity);
        assert(service->resume.valid && service->resume.selector == source_services[index].resume.selector);
        assert(service->current_operation == source_services[index].current_operation);
        assert(service->last_refusal.code == AMA_BUSY && service->last_refusal.message_length == 3);
    }
    assert(mapped_cell(external_leaf)->metadata.service.activity == AMA_INSTANCE_RUNNING);
    assert(mapped_cell(external_root)->metadata.service.activity == AMA_INSTANCE_RUNNING);
    mapped_cell(external_leaf)->metadata.service.activity = AMA_INSTANCE_READY;
    mapped_cell(external_root)->metadata.service.activity = AMA_INSTANCE_READY;
    AmaStatus status = ama_object_destroy(&world, source_root);
    assert(status.code == AMA_OK);
    status = ama_object_destroy(&world, external_leaf);
    assert(status.code == AMA_OK);
    status = ama_object_destroy(&world, external_root);
    assert(status.code == AMA_OK);
    destroy_objects();
}
