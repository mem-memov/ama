/** @file
 * @brief Compare explicit tree geometry with independently enumerated physical destinations.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include "../../../../../../tools/project/source/mapping/tree.h"
/** Exact original path lookup keeps source bytes attached to their proposed placement. */
static const SourceTreePlacementEntry *placement_entry(const SourceTreePlacement *placement, const char *path) {
    for (const SourceTreePlacementEntry *entry = placement->entries; entry; entry = entry->next) {
        if (!strcmp(entry->original->file.path, path)) { return entry; }
    }
    assert(false); return NULL;
}
/** Module renaming affects only the main stem, dot companions and complete plus subtree. */
static void it_places_module_stems_companions_and_subtrees_with_a_consistent_clone_relation(void) {
    prepare(); assert(source_tree_selection_read(&selection, &capture, main_path) == SOURCE_TREE_OK);
    SourceTreePlacement placement; source_tree_placement_init(&placement, sources.allocator);
    assert(source_tree_placement_build(&placement, &selection, "/mapped/copy") == SOURCE_PLACEMENT_OK && placement.ready);
    const char *const expected[] = {"/mapped/copy", "/mapped/copy+", "/mapped/copy+/child",
        "/mapped/copy+/empty", "/mapped/copy+/opaque.data", "/mapped/copy.ini"};
    size_t count = 0;
    for (const SourceTreePlacementEntry *entry = placement.entries; entry; entry = entry->next) {
        assert(count < sizeof(expected) / sizeof(*expected) && !strcmp(entry->path, expected[count])); count++;
    }
    assert(count == sizeof(expected) / sizeof(*expected));
    source_clone_map_init(&map, sources.allocator, &capture);
    const ModuleSource *parent = original("/vendor/pkg/v1.0.0/src/parent");
    const ModuleSource *child = original("/vendor/pkg/v1.0.0/src/parent/child");
    const SourceCloneMapping *mapping = NULL;
    assert(source_clone_map_add(&map, parent, placement_entry(&placement, source_path(parent->text))->path,
        "/vendor/target/v2.0.0/src/copy", &mapping) == SOURCE_CLONE_OK);
    assert(source_clone_map_add(&map, child, placement_entry(&placement, source_path(child->text))->path,
        "/vendor/target/v2.0.0/src/copy/child", &mapping) == SOURCE_CLONE_OK);
    assert(source_clone_map_seal(&map) == SOURCE_CLONE_OK);
    source_clone_map_destroy(&map); source_tree_placement_destroy(&placement); finish();
}
/** Directory placement preserves relative names; root directories do not imply a sibling main. */
static void it_places_bare_and_empty_directories_at_the_exact_requested_location(void) {
    const char *operands[] = {subtree_path, "generated/project/packages/vendor/pkg/v1.0.0/src/parent+/empty"};
    const char *const expected[] = {"/mapped/namespace", "/mapped/namespace/child", "/mapped/namespace/empty", "/mapped/namespace/opaque.data"};
    for (size_t choice = 0; choice < 2; choice++) {
        prepare(); assert(source_tree_selection_read(&selection, &capture, operands[choice]) == SOURCE_TREE_OK);
        SourceTreePlacement placement; source_tree_placement_init(&placement, sources.allocator);
        assert(source_tree_placement_build(&placement, &selection, "/mapped/namespace") == SOURCE_PLACEMENT_OK);
        size_t count = 0;
        for (const SourceTreePlacementEntry *entry = placement.entries; entry; entry = entry->next) {
            assert(count < (choice ? 1u : 4u) && !strcmp(entry->path, expected[count])); count++;
        }
        assert(count == (choice ? 1u : 4u));
        size_t before = allocation_count;
        assert(source_tree_placement_build(&placement, &selection, "/other") == SOURCE_PLACEMENT_INVALID && placement.ready);
        assert(allocation_count == before); source_tree_placement_destroy(&placement); finish();
    }
}
/** Every physical-name/row allocation failure leaves an owned unready prefix without source changes. */
static void it_retires_every_explicit_tree_placement_allocation_failure(void) {
    const char *operands[] = {main_path, subtree_path, "generated/project/packages/vendor/pkg/v1.0.0/src/parent+/empty"};
    for (size_t choice = 0; choice < 3; choice++) {
        size_t count = 0;
        for (size_t failure = 0; !failure || failure <= count; failure++) {
            prepare(); assert(source_tree_selection_read(&selection, &capture, operands[choice]) == SOURCE_TREE_OK);
            SourceTreePlacement placement; source_tree_placement_init(&placement, sources.allocator);
            size_t before = allocation_count; if (failure) { failure_at = before + failure; }
            SourcePlacementStatus status = source_tree_placement_build(&placement, &selection, "/mapped/target");
            if (!failure) { assert(status == SOURCE_PLACEMENT_OK); count = allocation_count - before; assert(count); }
            else { assert(status == SOURCE_PLACEMENT_MEMORY && !placement.ready && selection.valid); }
            failure_at = 0; source_tree_placement_destroy(&placement); finish();
        }
    }
}
/** Raw captured ignored versions and invalid destination forms cannot become usable placements. */
static void it_refuses_unadmitted_tree_domains_and_invalid_module_destination_forms(void) {
    prepare();
    assert(source_tree_selection_read(&selection, &capture, "generated/project/dependencies/vendor/pkg/v1.0.0/src") == SOURCE_TREE_OK);
    SourceTreePlacement placement; source_tree_placement_init(&placement, sources.allocator);
    assert(source_tree_placement_build(&placement, &selection, "/mapped/target") == SOURCE_PLACEMENT_DOMAIN && !placement.ready);
    assert(placement.domain_status == SOURCE_TREE_DOMAIN_UNSELECTED_VERSION && placement.failed_path);
    source_tree_placement_destroy(&placement); finish();
    const char *destinations[] = {"relative", "", "/", "/mapped/file/"};
    for (size_t index = 0; index < 4; index++) {
        prepare(); assert(source_tree_selection_read(&selection, &capture, main_path) == SOURCE_TREE_OK);
        source_tree_placement_init(&placement, sources.allocator);
        assert(source_tree_placement_build(&placement, &selection, destinations[index]) == SOURCE_PLACEMENT_INVALID && !placement.ready);
        source_tree_placement_destroy(&placement); finish();
    }
}
