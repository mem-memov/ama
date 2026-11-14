/** @file
 * @brief Check every acquired ancestor and restoration step without executing generated C.
 */
/** Two fixed selected frames retain exactly the unfinished path, with no allocation or copied payload. */
static void it_keeps_each_ancestor_acquired_and_restores_it_without_allocating(void) {
    setup_tree();
    bool refused = true;
    size_t allocations = allocation_count;
    failure_at = allocations + 1;
    AmaStatus status = ama_program_acquire(&program, nodes[0], &read_child, &refused);
    assert(status.code == AMA_OK && !refused);
    for (size_t index = 0; index < 2; index++) {
        status = ama_program_acquire_result(&program, nodes[0], &read_child,
            offsetof(TreeNode, frames) + index * sizeof(AmaProgramFrame), &refused);
        assert(status.code == AMA_OK && !refused);
        for (size_t ancestor = 1; ancestor <= index + 2; ancestor++) {
            assert(node_cell(ancestor)->metadata.service.activity == AMA_INSTANCE_ACQUIRED);
        }
        assert(ama_handle_equal(node_cell(0)->metadata.service.outgoing, nodes[index + 2]));
    }
    for (size_t index = 2; index != 0; index--) {
        status = ama_program_release_result(&program, nodes[0],
            offsetof(TreeNode, frames) + (index - 1) * sizeof(AmaProgramFrame), false);
        assert(status.code == AMA_OK && !node_value(0)->frames[index - 1].active);
        assert(ama_handle_equal(node_cell(0)->metadata.service.outgoing, nodes[index]));
        assert(node_cell(index + 1)->metadata.service.activity == AMA_INSTANCE_READY);
    }
    status = ama_program_release(&program, nodes[0]);
    assert(status.code == AMA_OK && allocation_count == allocations);
    failure_at = 0;
    finish_tree();
}
