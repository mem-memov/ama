/** @file
 * @brief Distinguish explicitly borrowed copy authority from unrelated or destructive acquisitions.
 */
/** Only an explicitly authorized nested copy can use an already acquired tree; it cannot release its outer owner's protection. */
static void it_borrows_only_explicit_copy_authority_and_preserves_the_outer_lease(void) {
    setup_preflight();
    world.operation_active = true;
    AmaObjectOperation outer;
    start_protection(&outer, NULL);
    AmaStatus status = ama_object_preflight(&outer, protected_nodes[0], true, false);
    assert(status.code == AMA_OK);
    size_t records = 0;
    for (const AmaObjectProtection *record = outer.protected_first; record != NULL; record = record->next) {
        assert(record->copy && record->restore);
        records++;
    }
    assert(records == 9);
    for (size_t index = 0; index < 9; index++) {
        assert(preflight_cell(protected_nodes[index])->metadata.service.activity == AMA_INSTANCE_ACQUIRED);
    }
    AmaObjectOperation outsider;
    start_protection(&outsider, NULL);
    status = ama_object_preflight(&outsider, protected_nodes[0], true, false);
    assert(status.code == AMA_BUSY && outsider.protected_first == NULL);
    end_protection(&outsider);
    AmaObjectOperation nested;
    start_protection(&nested, outer.protected_first);
    status = ama_object_preflight(&nested, protected_nodes[0], false, false);
    assert(status.code == AMA_BUSY && nested.protected_first == NULL);
    status = ama_object_preflight(&nested, protected_nodes[0], true, false);
    assert(status.code == AMA_OK);
    records = 0;
    for (const AmaObjectProtection *record = nested.protected_first; record != NULL; record = record->next) {
        assert(record->copy && !record->restore);
        records++;
    }
    assert(records == 9);
    end_protection(&nested);
    for (size_t index = 0; index < 9; index++) {
        assert(preflight_cell(protected_nodes[index])->metadata.service.activity == AMA_INSTANCE_ACQUIRED);
    }
    end_protection(&outer);
    world.operation_active = false;
    finish_preflight();
}
