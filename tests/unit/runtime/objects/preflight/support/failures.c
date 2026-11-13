/** @file
 * @brief Verify complete restoration after every allocation prefix and malformed owned-tree edge.
 */
/** Fail each capability/protection allocation, including partially acquired trees, without losing saved positions or refusal history. */
static void it_restores_every_service_field_after_each_partial_preflight_allocation_failure(void) {
    setup_preflight();
    world.operation_active = true;
    AmaObjectOperation operation;
    start_protection(&operation, NULL);
    size_t before = allocation_count;
    AmaStatus status = ama_object_preflight(&operation, protected_nodes[0], true, false);
    assert(status.code == AMA_OK);
    size_t count = allocation_count - before;
    assert(count > 9);
    end_protection(&operation);
    world.operation_active = false;
    finish_preflight();
    bool partial = false;
    for (size_t failure = 1; failure <= count; failure++) {
        setup_preflight();
        world.operation_active = true;
        start_protection(&operation, NULL);
        failure_at = allocation_count + failure;
        status = ama_object_preflight(&operation, protected_nodes[0], true, false);
        assert(status.code == AMA_OUT_OF_MEMORY);
        partial |= operation.protected_first != NULL;
        end_protection(&operation);
        world.operation_active = false;
        failure_at = 0;
        finish_preflight();
    }
    assert(partial);
}
/** Owner mismatch, duplicate ownership and wrong nominal child type stop before native effects and restore every protected prefix. */
static void it_restores_protected_prefixes_when_owned_edges_are_invalid(void) {
    for (size_t shape = 0; shape < 3; shape++) {
        setup_preflight();
        RootValue *root = object_payload(protected_nodes[0]);
        AmaPoolCell *left = preflight_cell(root->left);
        AmaHandle original_owner = left->owner;
        AmaHandle original_right = root->right;
        if (shape == 0) { left->owner = unrelated; }
        else if (shape == 1) { root->right = root->left; }
        else { root->right.type_id = 99; }
        world.operation_active = true;
        AmaObjectOperation operation;
        start_protection(&operation, NULL);
        AmaStatus status = ama_object_preflight(&operation, protected_nodes[0], true, false);
        assert(status.code == (shape == 2 ? AMA_TYPE_MISMATCH : AMA_INVALID_STATE));
        assert(operation.protected_first != NULL);
        end_protection(&operation);
        world.operation_active = false;
        left->owner = original_owner;
        root->right = original_right;
        finish_preflight();
    }
}
