/** @file
 * @brief Verify exact byte preparation independent of edit arrival order and fail-at-N allocation.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
/** Enumerate all 24 edit orders against one independently specified byte sequence. */
static void it_applies_every_edit_permutation_to_the_same_exact_prepared_bytes(void) {
    const unsigned char expected[] = {'a', '<', 'X', 'Y', 'Z', '!', 'd', '\r', '\n'};
    for (unsigned a = 0; a < 4; a++) {
        for (unsigned b = 0; b < 4; b++) {
            for (unsigned c = 0; c < 4; c++) {
                for (unsigned d = 0; d < 4; d++) {
                    if (a == b || a == c || a == d || b == c || b == d || c == d) { continue; }
                    prepare(); model_edit(a); model_edit(b); model_edit(c); model_edit(d);
                    const SourceText *result = NULL;
                    assert(source_text_edits_apply(&edits, &sources, "prepared", &result) == SOURCE_EDITS_OK);
                    assert(edits.valid && edits.result == result && !strcmp(source_path(result), "prepared"));
                    assert(source_length(result) == sizeof(expected) && !memcmp(source_bytes(result), expected, sizeof(expected)));
                    assert(source_length(input) == sizeof(raw) && !memcmp(source_bytes(input), raw, sizeof(raw)));
                    assert(source_text_edits_apply(&edits, &sources, "again", &result) == SOURCE_EDITS_INVALID && edits.valid);
                    assert(source_text_edits_add(&edits, (SourceSpan){input, 0, 0}, NULL, 0) == SOURCE_EDITS_INVALID);
                    finish();
                }
            }
        }
    }
}
/** Empty inputs/outputs and no-edit copies preserve exact byte and ownership contracts. */
static void it_handles_empty_input_complete_deletion_and_unchanged_opaque_bytes(void) {
    for (unsigned choice = 0; choice < 3; choice++) {
        prepare_bytes(raw, choice == 0 ? 0 : sizeof(raw));
        if (choice == 1) { assert(source_text_edits_add(&edits, (SourceSpan){input, 0, sizeof(raw)}, NULL, 0) == SOURCE_EDITS_OK); }
        const SourceText *result = NULL;
        assert(source_text_edits_apply(&edits, &sources, "prepared", &result) == SOURCE_EDITS_OK);
        assert(source_length(result) == (choice == 2 ? sizeof(raw) : 0));
        if (choice == 2) { assert(!memcmp(source_bytes(result), raw, sizeof(raw))); }
        finish();
    }
}
/** Every measured output-buffer/source-owner allocation failure preserves the caller's output. */
static void it_retires_every_prepared_output_allocation_failure_and_invalid_apply_interface(void) {
    size_t count = 0;
    for (size_t failure = 0; !failure || failure <= count; failure++) {
        prepare(); for (unsigned index = 0; index < 4; index++) { model_edit(index); }
        size_t before = allocation_count; if (failure) { failure_at = before + failure; }
        const SourceText *result = input;
        SourceEditsStatus status = source_text_edits_apply(&edits, &sources, "prepared", &result);
        if (!failure) { assert(status == SOURCE_EDITS_OK); count = allocation_count - before; assert(count); }
        else { assert(status == SOURCE_EDITS_MEMORY && result == input && !edits.valid && !edits.result); }
        failure_at = 0; finish();
    }
    for (unsigned choice = 0; choice < 3; choice++) {
        prepare(); const SourceText *result = input;
        assert(source_text_edits_apply(&edits, choice == 0 ? NULL : &sources, choice == 1 ? NULL : "prepared", choice == 2 ? NULL : &result) == SOURCE_EDITS_INVALID);
        assert(result == input && !edits.valid); finish();
    }
}
