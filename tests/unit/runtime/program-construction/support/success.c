/** @file
 * @brief Verify declaration order, direct literal construction, self identity and native-before-reverse-children teardown.
 */
/** Real recipes initialize self fields at final private addresses, then publish one complete tree and destroy it in prescribed order. */
static void it_constructs_real_self_initializers_once_at_final_private_addresses(void) {
    setup_observed_construction();
    AmaHandle root;
    AmaStatus status = ama_object_create(&world, 2, NULL, &root);
    assert(status.code == AMA_OK && constructed == 3 && cloned == 0 && default_calls == 2 && literal_calls == 1);
    assert(issued_count == 3 && ama_handle_equal(issued[0], root));
    const uint64_t types[] = {2, 1, 1, 2, 1, 1, 2, 2, 2, 2};
    const size_t fields[] = {0, 0, 1, 1, 0, 1, 2, 3, 4, 5};
    assert(field_count == sizeof(fields) / sizeof(*fields));
    for (size_t index = 0; index < field_count; index++) {
        assert(field_types[index] == types[index] && field_ordinals[index] == fields[index]);
    }
    assert_constructed_self_tree(root);
    status = ama_object_destroy(&world, root);
    assert(status.code == AMA_OK && destroyed == 3);
    assert(destruction_order[0] == 3 && destruction_order[1] == 2 && destruction_order[2] == 42);
    destroy_objects();
}
/** Public program startup installs the canonical resolver and produces the same concrete self-reference fields. */
static void it_starts_the_program_with_the_same_typed_self_recipe_path(void) {
    setup_construction_program();
    AmaHandle root;
    AmaStatus status = ama_program_start(&program, 2, &root);
    assert(status.code == AMA_OK && constructed == 3 && cloned == 0);
    assert_constructed_self_tree(root);
    status = ama_program_stop(&program, root, status);
    assert(status.code == AMA_OK && constructed == destroyed);
    destroy_test();
}
/** Forming self does not acquire a Running instance, and failure publishes no borrowed output for stale/wrong-type/fresh/shared contexts. */
static void it_forms_self_without_acquiring_running_state_and_rejects_invalid_identity(void) {
    setup_construction_program();
    AmaHandle root;
    AmaStatus status = ama_program_start(&program, 2, &root);
    assert(status.code == AMA_OK);
    AmaPoolCell *cell = NULL;
    status = ama_pool_lookup(&root_type.pool, root, false, &cell);
    assert(status.code == AMA_OK);
    cell->metadata.service.activity = AMA_INSTANCE_RUNNING;
    AmaProgramValue value = {0};
    status = ama_program_evaluate(&program, root, NULL, &root_self, false, &value);
    assert(status.code == AMA_OK && ama_handle_equal(value.handle, root) && value.form == AMA_FIELD_REFERENCE);
    assert(cell->metadata.service.activity == AMA_INSTANCE_RUNNING && value.location == NULL);
    value.type_id = 99;
    status = ama_program_evaluate(&program, root, NULL, &leaf_self, false, &value);
    assert(status.code == AMA_TYPE_MISMATCH && value.type_id == 99);
    AmaConstantValue shared = {0};
    status = ama_program_evaluate(&program, root, &shared, &root_self, true, &value);
    assert(status.code == AMA_INVALID_STATE && value.type_id == 99);
    cell->metadata.service.activity = AMA_INSTANCE_READY;
    status = ama_object_destroy(&world, root);
    assert(status.code == AMA_OK);
    status = ama_program_evaluate(&program, root, NULL, &root_self, true, &value);
    assert(status.code == AMA_STALE_REFERENCE && value.type_id == 99);
    AmaHandle fresh;
    status = ama_pool_reserve(&root_type.pool, &fresh, &cell);
    assert(status.code == AMA_OK);
    status = ama_program_evaluate(&program, fresh, NULL, &root_self, true, &value);
    assert(status.code == AMA_INVALID_STATE && value.type_id == 99);
    status = ama_pool_release(&root_type.pool, fresh);
    assert(status.code == AMA_OK);
    destroy_objects();
}
