/** @file
 * @brief Exhaust measured material and review allocation failures while retaining unaccepted output and deterministic cleanup.
 * @see [T224 contract](../../../../../../../docs/implementation/source-operations.md)
 */
/** Every declared-material allocation failure stays owned and cannot publish a valid removal set. */
static void it_retires_every_explicit_asset_material_allocation_failure(void) {
    const FixtureMaterial profiles[] = {MATERIAL_NONE, MATERIAL_NATIVE_ASSET};
    for (size_t choice = 0; choice < 2; choice++) {
        size_t count = 0;
        for (size_t failure = 0; !failure || failure <= count; failure++) {
            begin_removal(profiles[choice], main_path); AcquisitionUnitEntry file_entry; asset(&file_entry, "asset\n");
            SourceDeclaredGroup declared = {.requester = unit("/vendor/pkg/v1.0.0/src/parent/child"), .files = &file_entry};
            size_t before = allocation_count;
            if (failure) { failure_at = before + failure; }
            SourceRemovalSelectionStatus status = source_removal_selection_build(&removal, &selection, &analysis, &declared);
            if (!failure) { assert(status == SOURCE_REMOVAL_SELECTION_OK); count = allocation_count - before; assert(count); }
            else { assert(status != SOURCE_REMOVAL_SELECTION_OK && !removal.valid); }
            failure_at = 0; end_removal(profiles[choice]);
        }
    }
}
/** Every material and combined review allocation failure keeps output unaccepted and owned. */
static void it_retires_all_material_and_review_allocation_failures(void) {
    begin_removal(MATERIAL_TESTS, main_path); size_t before = allocation_count;
    assert(source_removal_selection_build(&removal, &selection, &analysis, NULL) == SOURCE_REMOVAL_SELECTION_OK);
    size_t material = allocation_count - before; assert(material); end_removal(MATERIAL_TESTS);
    for (size_t index = 1; index <= material; index++) {
        begin_removal(MATERIAL_TESTS, main_path); failure_at = allocation_count + index;
        assert(source_removal_selection_build(&removal, &selection, &analysis, NULL) != SOURCE_REMOVAL_SELECTION_OK && !removal.valid);
        failure_at = 0; end_removal(MATERIAL_TESTS);
    }
    for (size_t index = 1; index <= 2; index++) {
        begin_removal(MATERIAL_TESTS, main_path);
        assert(source_removal_selection_build(&removal, &selection, &analysis, NULL) == SOURCE_REMOVAL_SELECTION_OK);
        SourceClientRoot root = {.definition = definition("/vendor/pkg/v1.0.0/src/parent/child")};
        SourceClientRootBlocker blocker = {0};
        SourceRemovalReview sentinel = {.clients = &blocker}, review = sentinel;
        failure_at = allocation_count + index;
        assert(source_removal_selection_review(&removal, NULL, &root, &review) == SOURCE_REMOVAL_OUT_OF_MEMORY);
        assert(review.clients == sentinel.clients && review.consumers == sentinel.consumers);
        failure_at = 0; end_removal(MATERIAL_TESTS);
    }
}
