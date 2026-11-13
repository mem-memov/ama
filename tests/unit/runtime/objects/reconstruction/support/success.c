/** @file
 * @brief Exercise repeated final-identity bank swaps without moving native resources or retaining shadow identities.
 */
/** Self always names the retained destination, children name their fresh identities, and final native addresses survive publication. */
static void it_keeps_final_self_identity_and_native_addresses_through_default_and_literal_commit(void) {
    for (size_t mode = 0; mode < 2; mode++) {
        setup_reconstruction();
        AmaPoolCell *cell = reconstruction_cell(destination);
        AmaHandle identity = destination;
        const unsigned char byte = 7;
        AmaRecipe literal_recipe = {.constructor = AMA_CONSTRUCT_LITERAL, .literal = {&byte, 1}};
        for (size_t iteration = 0; iteration < 4; iteration++) {
            PreviousTree previous = snapshot_tree();
            size_t before = constructed;
            AmaStatus status = reconstruct(mode == 0 ? NULL : &literal_recipe);
            assert(status.code == AMA_OK && constructed == before + 3 && cloned == 0);
            assert(ama_handle_equal(destination, identity) && reconstruction_cell(destination) == cell);
            assert_constructed_self_tree(destination);
            RootValue *root = object_payload(destination);
            assert(root != previous.storage && (uintptr_t)root->native.resource != previous.resource_identity);
            if (mode == 1) { assert(root->native.number == 7); }
            assert(ama_handle_equal(reconstruction_cell(root->left)->owner, destination));
            assert(ama_handle_equal(reconstruction_cell(root->right)->owner, destination));
            assert(cell->metadata.service.activity == AMA_INSTANCE_READY && !cell->metadata.service.resume.valid);
            AmaPoolCell *old = NULL;
            status = ama_pool_lookup(&leaf_type.pool, previous.left, true, &old);
            assert(status.code == AMA_STALE_REFERENCE);
            status = ama_pool_lookup(&leaf_type.pool, previous.right, true, &old);
            assert(status.code == AMA_STALE_REFERENCE);
            AmaProgramValue self = {0};
            status = ama_program_evaluate(&program, destination, NULL, &root_self, false, &self);
            assert(status.code == AMA_OK && ama_handle_equal(self.handle, destination));
        }
        finish_reconstruction();
        assert(constructed == destroyed);
    }
}
