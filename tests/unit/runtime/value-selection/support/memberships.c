/** @file
 * @brief Exercise indexed reference replacement through the actual store boundary.
 */
/** Scalar reference assignment at an indexed receiver must retire its occurrence,
 * while identical handles keep it and stale/empty targets remain stored data.
 */
static void it_retires_only_changed_indexed_reference_occurrences(void) {
    setup_selection();
    RootValue *storage = object_payload(root);
    AmaMembership old, fresh;
    AmaStatus status = ama_membership_insert(&storage->references, 0, storage->right, &old);
    assert(status.code == AMA_OK);
    LeafValue *index_value = object_payload(storage->left);
    index_value->native.number = 0;
    program_types[0].index = decode_index;
    decoder_failure = false;
    const AmaProgramAccess index = {.field = 0};
    const AmaProgramAccess receiver = {.field = 5, .index = &index};
    AmaProgramValue value = {.type_id = 1, .form = AMA_FIELD_REFERENCE, .handle = storage->right};
    status = ama_program_transfer(&program, root, &receiver, &value);
    assert(status.code == AMA_OK);
    const AmaMembershipRecord *record = NULL;
    status = ama_membership_lookup(&storage->references, old, &record);
    assert(status.code == AMA_OK);
    value.handle = (AmaHandle){1, 999, 2};
    status = ama_program_transfer(&program, root, &receiver, &value);
    assert(status.code == AMA_OK && storage->references.count == 1);
    status = ama_membership_lookup(&storage->references, old, &record);
    assert(status.code == AMA_STALE_REFERENCE);
    AmaHandle selected;
    status = ama_membership_at(&storage->references, 0, &fresh, &selected);
    assert(status.code == AMA_OK && ama_handle_equal(selected, value.handle));
    value.handle = (AmaHandle){0};
    status = ama_program_transfer(&program, root, &receiver, &value);
    assert(status.code == AMA_OK);
    status = ama_membership_lookup(&storage->references, fresh, &record);
    assert(status.code == AMA_STALE_REFERENCE);
    assert(ama_handle_list_element(&storage->references, 0)->generation == 0);
    finish_selection();
}

/** An index operand is itself a checked value. Empty/stale operands cannot
 * invoke native conversion or overwrite a previously selected caller value.
 */
static void it_rejects_absent_index_operands_without_decoding_or_changing_outputs(void) {
    setup_selection();
    RootValue *storage = object_payload(root);
    AmaMembership identity;
    AmaStatus status = ama_membership_insert(&storage->references, 0, storage->right, &identity);
    assert(status.code == AMA_OK);
    program_types[0].index = decode_index;
    decodes = 0;
    const AmaProgramAccess index = {.field = 3};
    const AmaProgramAccess access = {.field = 5, .index = &index};
    AmaProgramValue output = {.type_id = 99};
    bool refused = true;
    status = ama_program_select_target(&program, root, &access, &output, &refused);
    assert(status.code == AMA_EMPTY_REFERENCE && !refused && decodes == 0 && output.type_id == 99);
    storage->selected = (AmaHandle){1, 999, 1};
    status = ama_program_select_target(&program, root, &access, &output, &refused);
    assert(status.code == AMA_STALE_REFERENCE && !refused && decodes == 0 && output.type_id == 99);
    finish_selection();
}
