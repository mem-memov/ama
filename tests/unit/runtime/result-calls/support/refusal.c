/** @file
 * @brief Preserve parent authority on refused selection and reject malformed service offsets.
 */
/** Busy/empty references refuse before frame activation; invalid offsets and an
 * inactive normal release are technical failures that preserve caller storage.
 */
static void it_refuses_before_parking_and_checks_frame_storage(void) {
    setup_tree();
    bool refused;
    AmaStatus status = ama_program_acquire(&program, nodes[0], &read_child, &refused);
    assert(status.code == AMA_OK && !refused);
    node_cell(2)->metadata.service.activity = AMA_INSTANCE_RUNNING;
    status = ama_program_acquire_result(&program, nodes[0], &read_child, offsetof(TreeNode, frames), &refused);
    assert(status.code == AMA_BUSY && refused && !node_value(0)->frames[0].active);
    assert(ama_handle_equal(node_cell(0)->metadata.service.outgoing, nodes[1]));
    node_cell(2)->metadata.service.activity = AMA_INSTANCE_READY;
    node_value(1)->child = (AmaHandle){0};
    status = ama_program_acquire_result(&program, nodes[0], &read_child, offsetof(TreeNode, frames), &refused);
    assert(status.code == AMA_EMPTY_REFERENCE && refused && !node_value(0)->frames[0].active);
    AmaProgramFrame sentinel = {0};
    AmaProgramFrame *output = &sentinel;
    status = ama_program_frame(&program, nodes[0], SIZE_MAX, &output);
    assert(status.code == AMA_INVALID_STATE && output == &sentinel);
    status = ama_program_frame(&program, nodes[0], offsetof(TreeNode, frames) + 1, &output);
    assert(status.code == AMA_INVALID_STATE && output == &sentinel);
    status = ama_program_release_result(&program, nodes[0], offsetof(TreeNode, frames), false);
    assert(status.code == AMA_INVALID_STATE);
    status = ama_program_release_result(&program, nodes[0], offsetof(TreeNode, frames), true);
    assert(status.code == AMA_OK);
    status = ama_program_release(&program, nodes[0]);
    assert(status.code == AMA_OK);
    finish_tree();
}
