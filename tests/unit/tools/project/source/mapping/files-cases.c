/** @file
 * @brief Verify opaque-file placement injectivity and final file/directory shape conflicts.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include "../../../../../../tools/project/source/mapping/files/files.h"
/** Construct independent byte observations, including embedded zeros and invalid UTF-8. */
static AcquisitionUnitEntry opaque_entry(const char *path, bool directory_entry, unsigned variant) {
    AcquisitionUnitEntry entry = {.directory = directory_entry,
        .file = {.path = path, .state = DELIVERY_FILE_PRESENT}};
    const unsigned char bytes[] = {0, 255, (unsigned char)variant};
    if (!directory_entry) { assert(source_manager_add(&sources, path, bytes, sizeof(bytes), &entry.file.content) == SOURCE_OK); }
    return entry;
}
/** Exhaust exact original/destination equality with shape and byte disagreement independently. */
static void it_checks_every_two_input_output_shape_and_byte_overlap_partition(void) {
    for (unsigned origin = 0; origin < 2; origin++) {
        for (unsigned destination = 0; destination < 2; destination++) {
            for (unsigned shape = 0; shape < 2; shape++) {
                for (unsigned changed = 0; changed < 2; changed++) {
                    setup_test(); SourceFileMap files; source_file_map_init(&files, sources.allocator);
                    AcquisitionUnitEntry first = opaque_entry("/original/a", false, 0);
                    AcquisitionUnitEntry second = opaque_entry(origin ? "/original/a" : "/original/b", shape != 0, changed);
                    const SourceFileMapping *row = NULL;
                    assert(source_file_map_add(&files, &first, "/final/a", &row) == SOURCE_FILE_MAP_OK);
                    const SourceFileMapping *saved = row; size_t before = allocation_count;
                    bool repeat = origin && destination && !shape && !changed;
                    bool allowed = repeat || (!origin && !destination);
                    SourceFileMapStatus status = source_file_map_add(&files, &second, destination ? "/final/a" : "/final/b", &row);
                    assert(status == (allowed ? SOURCE_FILE_MAP_OK : SOURCE_FILE_MAP_CONFLICT));
                    if (repeat) { assert(saved == row && allocation_count == before); }
                    if (!allowed) { assert(saved == row && files.conflict == saved && files.requester == &second); }
                    assert(source_file_map_seal(&files) == status);
                    source_file_map_destroy(&files); destroy_test();
                }
            }
        }
    }
}
/** File ancestors refuse in both insertion orders; component lookalikes and directories remain valid. */
static void it_rejects_final_file_ancestors_without_confusing_component_prefixes(void) {
    for (unsigned directory_entry = 0; directory_entry < 2; directory_entry++) {
        for (unsigned order = 0; order < 2; order++) {
            for (unsigned boundary = 0; boundary < 2; boundary++) {
                setup_test(); SourceFileMap files; source_file_map_init(&files, sources.allocator);
                AcquisitionUnitEntry entries[] = {opaque_entry("/original/a", directory_entry != 0, 0),
                    opaque_entry("/original/b", false, 0)};
                const char *destinations[] = {"/final/a", boundary ? "/final/ab/child" : "/final/a/child"};
                const SourceFileMapping *row = NULL;
                for (unsigned index = 0; index < 2; index++) {
                    unsigned selected = order ? 1 - index : index;
                    assert(source_file_map_add(&files, &entries[selected], destinations[selected], &row) == SOURCE_FILE_MAP_OK);
                }
                bool accepted = directory_entry || boundary; size_t before = allocation_count;
                assert(source_file_map_seal(&files) == (accepted ? SOURCE_FILE_MAP_OK : SOURCE_FILE_MAP_CONFLICT));
                assert(allocation_count == before && files.ready == accepted);
                if (!accepted) { assert(files.conflict && files.related && !files.conflict->original->directory); }
                source_file_map_destroy(&files); destroy_test();
            }
        }
    }
}
/** Actual complete tree placement retains opaque bytes and empty directories under an injective map. */
static void it_joins_complete_explicit_tree_file_maps_with_owned_keys_and_no_opaque_decoding(void) {
    prepare(); assert(source_tree_selection_read(&selection, &capture, main_path) == SOURCE_TREE_OK);
    SourceTreePlacement placement; source_tree_placement_init(&placement, sources.allocator);
    assert(source_tree_placement_build(&placement, &selection, "/final/copy") == SOURCE_PLACEMENT_OK);
    SourceFileMap files; source_file_map_init(&files, sources.allocator);
    assert(source_file_map_tree(&files, &placement) == SOURCE_FILE_MAP_OK);
    size_t count = 0;
    for (const SourceFileMapping *row = files.first; row; row = row->next) {
        const SourceTreePlacementEntry *expected = placement_entry(&placement, row->source);
        assert(!strcmp(row->destination, expected->path) && row->original == expected->original);
        assert(row->source != row->original->file.path && row->destination != expected->path); count++;
    }
    assert(count == 6); size_t before = allocation_count;
    assert(source_file_map_tree(&files, &placement) == SOURCE_FILE_MAP_OK && allocation_count == before);
    assert(source_file_map_seal(&files) == SOURCE_FILE_MAP_OK);
    assert(source_file_map_tree(&files, &placement) == SOURCE_FILE_MAP_INVALID && files.ready);
    source_file_map_destroy(&files); source_tree_placement_destroy(&placement); finish();
}
/** All proposal allocations fail deterministically without publishing the failing row or output. */
static void it_retires_every_file_map_allocation_and_invalid_capture_without_acceptance(void) {
    for (size_t failure = 1; failure <= 3; failure++) {
        setup_test(); SourceFileMap files; source_file_map_init(&files, sources.allocator);
        AcquisitionUnitEntry entry = opaque_entry("/original/a", false, 0);
        SourceFileMapping sentinel = {0}; const SourceFileMapping *row = &sentinel;
        failure_at = allocation_count + failure;
        assert(source_file_map_add(&files, &entry, "/final/a", &row) == SOURCE_FILE_MAP_MEMORY);
        assert(row == &sentinel && !files.first && !files.ready);
        assert(source_file_map_seal(&files) == SOURCE_FILE_MAP_MEMORY);
        failure_at = 0; source_file_map_destroy(&files); destroy_test();
    }
    for (unsigned choice = 0; choice < 8; choice++) {
        setup_test(); SourceFileMap files; source_file_map_init(&files, sources.allocator);
        AcquisitionUnitEntry entry = opaque_entry("/original/a", false, 0);
        const SourceFileMapping *row = NULL; const char *destination = "/final/a";
        if (choice == 0) { entry.file.state = DELIVERY_FILE_ABSENT; }
        if (choice == 1) { entry.file.content = NULL; }
        if (choice == 2) { entry.directory = true; }
        if (choice == 3) { destination = "relative"; }
        if (choice == 4) { destination = "/"; }
        if (choice == 5) { destination = "/final/a/"; }
        if (choice == 6) { entry.file.path = NULL; }
        assert(source_file_map_add(&files, &entry, destination, choice == 7 ? NULL : &row) == SOURCE_FILE_MAP_INVALID);
        assert(!files.first && !row && source_file_map_seal(&files) == SOURCE_FILE_MAP_INVALID);
        source_file_map_destroy(&files); destroy_test();
    }
    setup_test(); SourceFileMap files; source_file_map_init(&files, sources.allocator);
    assert(source_file_map_seal(&files) == SOURCE_FILE_MAP_OK && files.ready);
    assert(source_file_map_seal(&files) == SOURCE_FILE_MAP_INVALID);
    source_file_map_destroy(&files); destroy_test();
    setup_test(); source_file_map_init(&files, sources.allocator);
    AcquisitionUnitEntry root = opaque_entry("/original", true, 0), child = opaque_entry("/original/child", false, 0);
    const SourceFileMapping *row = NULL;
    assert(source_file_map_add(&files, &root, "/", &row) == SOURCE_FILE_MAP_OK);
    assert(source_file_map_add(&files, &child, "/child", &row) == SOURCE_FILE_MAP_OK);
    assert(source_file_map_seal(&files) == SOURCE_FILE_MAP_OK);
    source_file_map_destroy(&files); destroy_test();
    setup_test(); source_file_map_init(&files, sources.allocator);
    SourceTreePlacement unready = {0};
    assert(source_file_map_tree(&files, &unready) == SOURCE_FILE_MAP_INVALID);
    assert(source_file_map_seal(&files) == SOURCE_FILE_MAP_INVALID);
    source_file_map_destroy(&files); destroy_test();
}
