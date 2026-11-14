/** @file
 * @brief Verify whole-list and containing-root replacement identity boundaries.
 */
/** Copies preserve target relationships but mint independent occurrence identities;
 * exact self-copy is effect-free and counter exhaustion precedes destruction.
 */
static void it_publishes_fresh_copy_memberships_and_invalidates_replaced_incarnations(void) {
    setup_transfer();
    const AmaProgramAccess access = {.field = 2};
    AmaMembership source_identity, old_identity, replacement_identity;
    AmaHandle target;
    AmaStatus status = ama_membership_at(source_value.list, 0, &source_identity, &target);
    assert(status.code == AMA_OK);
    status = ama_program_transfer(&program, destination, &access, &source_value);
    assert(status.code == AMA_OK);
    status = ama_membership_at(destination_value.list, 0, &old_identity, &target);
    assert(status.code == AMA_OK && ama_handle_equal(old_identity.owner, destination));
    const AmaMembershipRecord *record = NULL;
    status = ama_membership_lookup(destination_value.list, source_identity, &record);
    assert(status.code == AMA_STALE_REFERENCE);
    uint64_t incarnation = destination_value.list->incarnation;
    size_t allocations = allocation_count, copies = cloned;
    status = ama_program_transfer(&program, destination, &access, &destination_value);
    assert(status.code == AMA_OK && allocation_count == allocations && cloned == copies);
    assert(destination_value.list->incarnation == incarnation);
    status = ama_program_transfer(&program, destination, &access, &source_value);
    assert(status.code == AMA_OK && destination_value.list->incarnation == incarnation + 1);
    status = ama_membership_lookup(destination_value.list, old_identity, &record);
    assert(status.code == AMA_STALE_REFERENCE);
    status = ama_membership_at(destination_value.list, 0, &replacement_identity, &target);
    assert(status.code == AMA_OK);
    incarnation = destination_value.list->incarnation;
    status = ama_object_replace(&world, destination, source, false);
    assert(status.code == AMA_OK);
    RootValue *updated = object_payload(destination);
    assert(updated->branches.incarnation == incarnation + 1);
    status = ama_membership_lookup(&updated->branches, replacement_identity, &record);
    assert(status.code == AMA_STALE_REFERENCE);
    updated->branches.counter_limit = updated->branches.incarnation;
    copies = cloned;
    size_t drops = destroyed;
    status = ama_object_replace(&world, destination, source, false);
    assert(status.code == AMA_LIMIT_EXCEEDED && cloned == copies && destroyed == drops);
    assert(updated->branches.count == 2 && !world.failed);
    finish_transfer();
}
