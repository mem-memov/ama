/** @file
 * @brief Exhaust all typed reference substitutions in one scalar-owned three-instance tree.
 */
/** 5^3 leaf-target choices times 4^3 root-target choices exercise internal/self/peer, duplicate external, stale colliding and empty references. */
static void it_maps_all_scalar_reference_substitutions_by_complete_identity(void) {
    for (size_t combination = 0; combination < 8000; combination++) {
        setup_map();
        RootValue *root = object_payload(source_root);
        LeafValue *left = object_payload(root->left), *right = object_payload(root->right);
        const AmaHandle leaf_choices[] = {root->left, root->right, external_leaf, stale_leaf, {0}};
        const AmaHandle root_choices[] = {source_root, external_root, stale_root, {0}};
        AmaHandle *leaf_receivers[] = {&root->selected, &left->peer, &right->peer};
        AmaHandle *root_receivers[] = {&root->self, &left->root, &right->root};
        size_t choices = combination;
        for (size_t index = 0; index < 3; index++) { *leaf_receivers[index] = leaf_choices[choices % 5]; choices /= 5; }
        for (size_t index = 0; index < 3; index++) { *root_receivers[index] = root_choices[choices % 4]; choices /= 4; }
        assert(choices == 0);
        AmaHandle before_leaf[3], before_root[3];
        for (size_t index = 0; index < 3; index++) {
            before_leaf[index] = *leaf_receivers[index];
            before_root[index] = *root_receivers[index];
        }
        AmaHandle copy;
        AmaStatus status = ama_object_clone(&world, source_root, false, &copy);
        assert(status.code == AMA_OK && cloned == 3);
        assert_scalar_copy(copy);
        for (size_t index = 0; index < 3; index++) {
            assert(ama_handle_equal(*leaf_receivers[index], before_leaf[index]));
            assert(ama_handle_equal(*root_receivers[index], before_root[index]));
        }
        status = ama_object_destroy(&world, copy);
        assert(status.code == AMA_OK);
        finish_map();
    }
}
