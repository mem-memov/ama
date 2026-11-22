/** @file
 * @brief Compare every occurrence against an independent boundary table after source edits.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
/** Tables describe actual original/prepared byte boundaries, not implementation delta arithmetic. */
static void it_projects_all_original_occurrences_using_exact_replacement_and_insertion_boundaries(void) {
    const int starts[] = {0, 2, -1, 6, 7, -1, 7, 8, 9};
    const int ends[] = {0, 1, -1, 5, 7, -1, 7, 8, 9};
    prepare(); for (unsigned index = 0; index < 4; index++) { model_edit(index); }
    const SourceText *result = NULL;
    assert(source_text_edits_apply(&edits, &sources, "prepared", &result) == SOURCE_EDITS_OK);
    for (size_t a = 0; a <= sizeof(raw); a++) {
        for (size_t b = a; b <= sizeof(raw); b++) {
            SourceSpan projected = {input, 99, 99}; int start = starts[a], end = a == b ? starts[b] : ends[b];
            bool expected = start >= 0 && end >= 0 && start <= end;
            size_t before = allocation_count;
            assert(source_text_edits_project(&edits, (SourceSpan){input, a, b}, &projected) == expected);
            assert(allocation_count == before);
            if (expected) { assert(projected.source == result && projected.start == (size_t)start && projected.end == (size_t)end); }
            else { assert(projected.source == input && projected.start == 99 && projected.end == 99); }
        }
    }
    finish();
}
/** Prefix/suffix insertions stay outside the original full nonempty occurrence. */
static void it_preserves_outside_boundary_insertions_and_requires_exact_prepared_owners(void) {
    prepare(); SourceSpan projected = {0};
    assert(!source_text_edits_project(&edits, (SourceSpan){input, 0, 8}, &projected));
    assert(source_text_edits_add(&edits, (SourceSpan){input, 0, 0}, (const unsigned char *)"pre", 3) == SOURCE_EDITS_OK);
    assert(source_text_edits_add(&edits, (SourceSpan){input, 8, 8}, (const unsigned char *)"post", 4) == SOURCE_EDITS_OK);
    const SourceText *result = NULL;
    assert(source_text_edits_apply(&edits, &sources, "prepared", &result) == SOURCE_EDITS_OK);
    assert(source_text_edits_project(&edits, (SourceSpan){input, 0, 8}, &projected));
    assert(projected.start == 3 && projected.end == 11 && projected.source == result);
    assert(!source_text_edits_project(&edits, (SourceSpan){result, 0, 8}, &projected));
    assert(!source_text_edits_project(&edits, (SourceSpan){input, 0, 9}, &projected));
    assert(!source_text_edits_project(&edits, (SourceSpan){input, 0, 8}, NULL));
    finish();
}
