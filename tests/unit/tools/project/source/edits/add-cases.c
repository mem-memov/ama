/** @file
 * @brief Exhaust finite range geometry and exact repeat/conflict ownership boundaries.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
/** Exhaust all pairs of closed endpoint choices over four bytes with two replacement values. */
static void it_exhausts_all_finite_range_and_insertion_overlap_partitions(void) {
    for (size_t a = 0; a <= 4; a++) {
        for (size_t b = a; b <= 4; b++) {
            for (size_t c = 0; c <= 4; c++) {
                for (size_t d = c; d <= 4; d++) {
                    for (unsigned changed = 0; changed < 2; changed++) {
                        prepare(); unsigned char first = 'x', second = changed ? 'y' : 'x';
                        assert(source_text_edits_add(&edits, (SourceSpan){input, a, b}, &first, 1) == SOURCE_EDITS_OK);
                        bool exact = a == c && b == d, occupied = false;
                        for (size_t cell = 0; cell < 4; cell++) {
                            if (a <= cell && cell < b && c <= cell && cell < d) { occupied = true; }
                        }
                        for (size_t point = 0; point <= 4; point++) {
                            if ((a == b && point == a && c < point && point < d)
                                || (c == d && point == c && a < point && point < b)) { occupied = true; }
                        }
                        bool allowed = exact ? !changed : !occupied;
                        size_t before = allocation_count;
                        assert(source_text_edits_add(&edits, (SourceSpan){input, c, d}, &second, 1)
                            == (allowed ? SOURCE_EDITS_OK : SOURCE_EDITS_CONFLICT));
                        if (exact && !changed) { assert(allocation_count == before); }
                        if (!allowed) {
                            assert(edits.conflict && !edits.valid);
                            const SourceText *result = input;
                            assert(source_text_edits_apply(&edits, &sources, "prepared", &result) == SOURCE_EDITS_CONFLICT && result == input);
                            assert(source_text_edits_add(&edits, (SourceSpan){input, a, b}, &first, 1) == SOURCE_EDITS_CONFLICT);
                        }
                        finish();
                    }
                }
            }
        }
    }
}
/** Owned replacement bytes survive caller changes and every distinct-row allocation may fail. */
static void it_owns_replacements_and_retires_each_add_allocation_failure(void) {
    prepare(); unsigned char replacement = 'x';
    assert(source_text_edits_add(&edits, (SourceSpan){input, 0, 1}, &replacement, 1) == SOURCE_EDITS_OK);
    replacement = 'y'; assert(edits.first->bytes[0] == 'x'); finish();
    for (unsigned deletion = 0; deletion < 2; deletion++) {
        for (size_t failure = 1; failure <= (deletion ? 1u : 2u); failure++) {
            prepare(); failure_at = allocation_count + failure;
            assert(source_text_edits_add(&edits, (SourceSpan){input, 0, 1}, deletion ? NULL : &replacement, deletion ? 0 : 1) == SOURCE_EDITS_MEMORY);
            assert(!edits.first && !edits.valid); const SourceText *result = input;
            assert(source_text_edits_apply(&edits, &sources, "prepared", &result) == SOURCE_EDITS_MEMORY && result == input);
            failure_at = 0; finish();
        }
    }
    for (unsigned earlier = 0; earlier < 2; earlier++) {
        for (size_t failure = 1; failure <= 2; failure++) {
            prepare(); unsigned char bytes = 'x';
            assert(source_text_edits_add(&edits, (SourceSpan){input, 3, 4}, &bytes, 1) == SOURCE_EDITS_OK);
            const SourceTextEdit *saved = edits.first;
            failure_at = allocation_count + failure;
            SourceSpan next = {input, earlier ? 0 : 6, earlier ? 1 : 7};
            assert(source_text_edits_add(&edits, next, &bytes, 1) == SOURCE_EDITS_MEMORY);
            assert(edits.first == saved && !saved->next && saved->bytes[0] == 'x');
            failure_at = 0; finish();
        }
    }
}
/** Foreign/bad ranges and impossible byte extents fail without replacement publication. */
static void it_refuses_invalid_edit_interfaces_and_unrepresentable_lengths(void) {
    for (unsigned choice = 0; choice < 7; choice++) {
        prepare(); SourceSpan span = {input, 0, 1}; const unsigned char *bytes = raw; size_t length = 1;
        if (choice == 0) { span.source = NULL; }
        if (choice == 1) { span.start = 2; span.end = 1; }
        if (choice == 2) { span.end = sizeof(raw) + 1; }
        if (choice == 3) { bytes = NULL; }
        if (choice == 4) { length = SIZE_MAX; }
        if (choice == 5) { edits.original = NULL; }
        if (choice == 6) {
            const SourceText *foreign = NULL;
            assert(source_manager_add(&sources, "foreign", raw, sizeof(raw), &foreign) == SOURCE_OK); span.source = foreign;
        }
        assert(source_text_edits_add(&edits, span, bytes, length) == (choice == 4 ? SOURCE_EDITS_LIMIT : SOURCE_EDITS_INVALID));
        assert(!edits.first && !edits.valid); finish();
    }
}
