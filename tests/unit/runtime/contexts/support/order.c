/**
 * @file
 * @brief Exercise observable root cleanup order across allocation and lifecycle changes.
 *
 * Included by the context leaf after its guarded fixtures. Exact destructor traces
 * distinguish creation order from type/slot scans and verify unchanged-root replacement.
 */

/** Mix nominal types and reuse an explicitly disposed older root's slot.
 * The new occupant must become newest without reviving the removed identity.
 */
static void it_orders_mixed_root_types_after_early_disposal_and_slot_reuse(void) {
    setup_contexts();
    uint64_t identity = create_context();
    AmaContextLease lease = {0};
    AmaContextView view = enter_context(identity, &lease);
    AmaContextReference first = create_root(&lease, view);
    AmaHandle leaf = {0};
    AmaStatus status = ama_object_create(view.world, 1, NULL, &leaf);
    assert(status.code == AMA_OK);
    (void)create_root(&lease, view);
    status = ama_object_destroy(view.world, first.local);
    assert(status.code == AMA_OK);
    AmaContextReference newest = create_root(&lease, view);
    assert(newest.local.slot_id == first.local.slot_id);
    assert(newest.local.generation != first.local.generation);
    assert(view.world->first_root->root_type_id == 1);
    leave_context(&lease);
    status = ama_context_dispose(&registry, identity);
    assert(status.code == AMA_OK && destroyed == 11);
    const size_t expected[] = {4, 3, 2, 11, 10, 9, 8, 7, 6, 5, 37};
    assert(memcmp(destruction_order, expected, sizeof(expected)) == 0);
    finish_contexts();
}

/** Replacement destroys old contents now but retains the root's original list position.
 * A distinguishable replacement value makes its later destruction observable.
 */
static void it_keeps_replaced_root_identity_in_its_original_cleanup_position(void) {
    setup_contexts();
    uint64_t identity = create_context();
    AmaContextLease lease = {0};
    AmaContextView view = enter_context(identity, &lease);
    AmaContextReference first = create_root(&lease, view), second = create_root(&lease, view);
    AmaPoolCell *original = view.world->first_root;
    AmaStatus status = ama_object_replace(view.world, first.local, second.local, false);
    assert(status.code == AMA_OK && view.world->first_root == original);
    assert(view.world->last_root == original->root_next);
    root_value(&lease, first)->native.number = 91;
    leave_context(&lease);
    status = ama_context_dispose(&registry, identity);
    assert(status.code == AMA_OK && destroyed == 10);
    const size_t expected[] = {4, 3, 2, 7, 6, 5, 91, 6, 5, 37};
    assert(memcmp(destruction_order, expected, sizeof(expected)) == 0);
    finish_contexts();
}

/** A failed destructive replacement retains a finalized destination slot for terminal cleanup.
 * Its invalid public identity must not hide that outstanding slot from the root chain.
 */
static void it_releases_a_retained_root_slot_after_destructive_replacement_failure(void) {
    setup_contexts();
    uint64_t identity = create_context();
    AmaContextLease lease = {0};
    AmaContextView view = enter_context(identity, &lease);
    AmaContextReference first = create_root(&lease, view), second = create_root(&lease, view);
    AmaPoolCell *original = view.world->first_root;
    failed_destructor = destroyed + 1;
    AmaStatus status = ama_object_replace(view.world, first.local, second.local, false);
    assert(status.code == AMA_IO_FAILURE && view.world->failed);
    assert(view.world->first_root == original && !original->metadata.identity.live);
    assert(original->metadata.lifetime.phase == AMA_LIFETIME_FINALIZED);
    assert(view.world->last_root == original->root_next && destroyed == 6);
    leave_context(&lease);
    status = ama_context_dispose(&registry, identity);
    assert(status.code == AMA_IO_FAILURE && destroyed == 10 && live_allocations == 0);
    const size_t expected[] = {4, 3, 2, 7, 6, 5, 7, 6, 5, 37};
    assert(memcmp(destruction_order, expected, sizeof(expected)) == 0);
    failed_destructor = 0;
    finish_contexts();
}

/** Public cloning creates a newer independent root; private clone descendants do not.
 * Each native clone receives fresh address-sensitive storage through the real hook.
 */
static void it_registers_public_clones_after_their_source_roots(void) {
    setup_contexts();
    uint64_t identity = create_context();
    AmaContextLease lease = {0};
    AmaContextView view = enter_context(identity, &lease);
    AmaContextReference first = create_root(&lease, view);
    AmaHandle clone = {0};
    AmaStatus status = ama_object_clone(view.world, first.local, false, &clone);
    assert(status.code == AMA_OK);
    AmaContextReference reference = {0};
    status = ama_context_reference(&lease, clone, &reference);
    assert(status.code == AMA_OK);
    root_value(&lease, reference)->native.number = 81;
    assert(view.world->first_root->root_next == view.world->last_root);
    leave_context(&lease);
    status = ama_context_dispose(&registry, identity);
    assert(status.code == AMA_OK && destroyed == 7);
    const size_t expected[] = {81, 3, 2, 4, 3, 2, 37};
    assert(memcmp(destruction_order, expected, sizeof(expected)) == 0);
    finish_contexts();
}

/** Partial construction/clone failures unwind their child prefix without adding a root.
 * Existing roots remain registered for terminal cleanup, retaining the original failure.
 */
static void it_excludes_failed_create_and_clone_from_root_publication_order(void) {
    for (size_t clone = 0; clone < 2; clone++) {
        setup_contexts();
        uint64_t identity = create_context();
        AmaContextLease lease = {0};
        AmaContextView view = enter_context(identity, &lease);
        AmaContextReference first = create_root(&lease, view);
        AmaPoolCell *original = view.world->first_root;
        failed_constructor = constructed + 2;
        AmaHandle output = {0};
        AmaStatus status = clone ? ama_object_clone(view.world, first.local, false, &output)
            : ama_object_create(view.world, 2, NULL, &output);
        assert(status.code == AMA_INVALID_LITERAL && ama_handle_is_empty(output));
        assert(view.world->first_root == original && view.world->last_root == original);
        assert(original->root_previous == NULL && original->root_next == NULL);
        leave_context(&lease);
        status = ama_context_dispose(&registry, identity);
        assert(status.code == AMA_INVALID_LITERAL && destroyed == 5);
        assert(destruction_order[0] == (clone ? 2 : 5));
        const size_t expected[] = {4, 3, 2, 37};
        assert(memcmp(destruction_order + 1, expected, sizeof(expected)) == 0);
        failed_constructor = 0;
        finish_contexts();
    }
}

/** Give each root a distinct cleanup error while preserving the fixture's exact release.
 * The first root in prescribed reverse order must supply the primary status.
 */
static AmaStatus ordered_destroy_payload(void *address) {
    size_t number = ((NativeValue *)address)->number;
    AmaStatus status = destroy_payload(address);
    if (status.code != AMA_OK) { return status; }
    if (number == 7) { return ama_status_make(AMA_NATIVE_FAILURE, NULL, 0); }
    if (number == 4) { return ama_status_make(AMA_IO_FAILURE, NULL, 0); }
    return status;
}

/** Every destructor runs after the newest-root error, and older errors cannot replace it. */
static void it_continues_reverse_root_cleanup_after_distinct_destructor_errors(void) {
    setup_contexts();
    context_types[1].hooks.destroy = ordered_destroy_payload;
    uint64_t identity = create_context();
    AmaContextLease lease = {0};
    AmaContextView view = enter_context(identity, &lease);
    (void)create_root(&lease, view);
    (void)create_root(&lease, view);
    leave_context(&lease);
    AmaStatus status = ama_context_dispose(&registry, identity);
    assert(status.code == AMA_NATIVE_FAILURE && reports == 2 && destroyed == 7);
    const size_t expected[] = {7, 6, 5, 4, 3, 2, 37};
    assert(memcmp(destruction_order, expected, sizeof(expected)) == 0);
    finish_contexts();
}
