/** @file
 * @brief Verify direct intrinsic values with optional outer module acquisitions.
 */
/** Root services have no synthetic outgoing instance; a selected result parks
 * that empty context and restores it after ordinary target release.
 */
static void it_acquires_materialized_values_with_empty_or_retained_outer_contexts(void) {
    setup_tree();
    const size_t offset = offsetof(TreeNode, frames[0]);
    AmaProgramValue value = {.type_id = 1, .form = AMA_FIELD_REFERENCE, .handle = nodes[1]};
    bool refused = true;
    size_t before = allocation_count;
    AmaStatus status = ama_program_acquire_value(&program, nodes[0], &value, AMA_ACCESS_READ, offset, &refused);
    assert(status.code == AMA_OK && !refused && allocation_count == before);
    assert(node_value(0)->frames[0].active && ama_handle_is_empty(node_value(0)->frames[0].parent));
    assert(node_cell(1)->metadata.service.activity == AMA_INSTANCE_ACQUIRED);
    status = ama_program_release_result(&program, nodes[0], offset, false);
    assert(status.code == AMA_OK && ama_handle_is_empty(node_cell(0)->metadata.service.outgoing));
    assert(!node_value(0)->frames[0].active && node_cell(1)->metadata.service.activity == AMA_INSTANCE_READY);
    status = ama_program_acquire(&program, nodes[0], &read_child, &refused);
    assert(status.code == AMA_OK);
    value.handle = nodes[2];
    status = ama_program_acquire_value(&program, nodes[0], &value, AMA_ACCESS_READ, offset, &refused);
    assert(status.code == AMA_OK && !refused);
    assert(ama_handle_equal(node_value(0)->frames[0].parent, nodes[1]));
    status = ama_program_release_result(&program, nodes[0], offset, false);
    assert(status.code == AMA_OK && ama_handle_equal(node_cell(0)->metadata.service.outgoing, nodes[1]));
    assert(node_cell(1)->metadata.service.activity == AMA_INSTANCE_ACQUIRED);
    status = ama_program_release(&program, nodes[0]);
    assert(status.code == AMA_OK);
    value.handle = (AmaHandle){0};
    status = ama_program_acquire_value(&program, nodes[0], &value, AMA_ACCESS_ENTRY, offset, &refused);
    assert(status.code == AMA_EMPTY_REFERENCE && refused && !node_value(0)->frames[0].active);
    value.handle = nodes[0];
    status = ama_program_acquire_value(&program, nodes[0], &value, AMA_ACCESS_ENTRY, offset, &refused);
    assert(status.code == AMA_BUSY && refused && !node_value(0)->frames[0].active);
    value.type_id = 2;
    status = ama_program_acquire_value(&program, nodes[0], &value, AMA_ACCESS_ENTRY, offset, &refused);
    assert(status.code == AMA_INVALID_STATE && !refused && !node_value(0)->frames[0].active);
    assert(allocation_count == before);
    finish_tree();
}
