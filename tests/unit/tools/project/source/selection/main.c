/** @file
 * @brief Verify exact physical selection, opaque preservation and original main/companion coherence.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include "common.c"
/** Check exact original logical identity once, independent of physical basename spelling. */
static bool selected(const char *identity) {
    for (size_t index = 0; index < selection.count; index++) {
        if (!strcmp(selection.units[index]->source->logical_path, identity)) { return true; }
    }
    return false;
}
/** File and bare plus-directory selections have distinct module membership. */
static void it_distinguishes_module_files_from_bare_submodule_directories(void) {
    for (unsigned directory_selection = 0; directory_selection < 2; directory_selection++) {
        prepare();
        assert(source_tree_selection_read(&selection, &capture, directory_selection ? subtree_path : main_path) == SOURCE_TREE_OK);
        assert(selection.valid && selection.count == (directory_selection ? 1u : 2u));
        assert(selected("/vendor/pkg/v1.0.0/src/parent") == !directory_selection);
        assert(selected("/vendor/pkg/v1.0.0/src/parent/child") && !selected("/vendor/pkg/v1.0.0/src/parentish"));
        bool empty = false, opaque = false, configuration = false;
        for (const AcquisitionUnitEntry *entry = selection.members; entry; entry = entry->next) {
            if (strstr(entry->file.path, "/empty")) { empty = entry->directory; }
            if (strstr(entry->file.path, "/opaque.data")) { opaque = source_length(entry->file.content) == 13; }
            if (strstr(entry->file.path, "/parent.ini")) { configuration = true; }
            assert(!strstr(entry->file.path, "/parent+.data"));
        }
        assert(empty && opaque && configuration == !directory_selection);
        size_t before = allocation_count;
        assert(source_tree_selection_read(&selection, &capture, main_path) == SOURCE_TREE_INVALID && selection.valid);
        assert(allocation_count == before); finish();
    }
}
/** Directory projection retains complete raw physical membership, including no-module directories. */
static void it_preserves_source_area_and_empty_directory_membership_without_invented_seeds(void) {
    const char *paths[] = {"generated/project/packages/vendor/pkg/v1.0.0/src",
        "generated/project/packages/vendor/pkg/v1.0.0/src/parent+/empty"};
    for (size_t index = 0; index < 2; index++) {
        prepare(); assert(source_tree_selection_read(&selection, &capture, paths[index]) == SOURCE_TREE_OK);
        assert(selection.count == (index ? 0u : 3u));
        if (index) { assert(selection.members && selection.members->directory && !selection.members->next && !selection.units); }
        else { assert(selected("/vendor/pkg/v1.0.0/src/parentish")); }
        finish();
    }
}
/** A companion, outside directory, missing path or symlink cannot masquerade as a selected main. */
static void it_refuses_nonmodule_outside_missing_and_aliased_operands(void) {
    const char *paths[] = {"generated/project/packages/vendor/pkg/v1.0.0/src/parent.ini", "generated/other",
        "generated/project/missing", "generated/project/link"};
    const SourceTreeStatus expected[] = {SOURCE_TREE_NOT_MODULE, SOURCE_TREE_OUTSIDE, SOURCE_TREE_PHYSICAL, SOURCE_TREE_PHYSICAL};
    for (size_t index = 0; index < 4; index++) {
        prepare();
        if (index == 3) { assert(symlink("packages/vendor/pkg/v1.0.0/src/parent", paths[index]) == 0); }
        assert(source_tree_selection_read(&selection, &capture, paths[index]) == expected[index] && !selection.valid);
        finish(); if (index == 3) { assert(unlink(paths[index]) == 0); }
    }
}
/** Projection refuses disappeared/changed indexed bytes and changed companion membership. */
static void it_refuses_changed_original_units_in_file_and_directory_selections(void) {
    for (size_t change = 0; change < 4; change++) {
        prepare();
        const char *extra = "generated/project/packages/vendor/pkg/v1.0.0/src/parent.extra";
        if (change == 0) { file(main_path, "changed parent\n"); }
        if (change == 1) { file(extra, "new companion\n"); }
        if (change == 2) { assert(unlink("generated/project/packages/vendor/pkg/v1.0.0/src/parent.ini") == 0); }
        if (change == 3) { assert(unlink("generated/project/packages/vendor/pkg/v1.0.0/src/parent+/child") == 0); }
        const char *source = change == 3 ? subtree_path : main_path;
        assert(source_tree_selection_read(&selection, &capture, source) == SOURCE_TREE_CHANGED && !selection.valid);
        finish(); if (change == 1) { assert(unlink(extra) == 0); }
    }
}
/** Every measured path, traversal, byte and projection allocation failure stays destructible. */
static void it_retires_all_file_directory_and_empty_selection_allocation_failures(void) {
    const char *paths[] = {main_path, subtree_path, "generated/project/packages/vendor/pkg/v1.0.0/src/parent+/empty"};
    for (size_t choice = 0; choice < 3; choice++) {
        prepare(); size_t before = allocation_count;
        assert(source_tree_selection_read(&selection, &capture, paths[choice]) == SOURCE_TREE_OK);
        size_t count = allocation_count - before; assert(count); finish();
        for (size_t failure = 1; failure <= count; failure++) {
            prepare(); failure_at = allocation_count + failure;
            assert(source_tree_selection_read(&selection, &capture, paths[choice]) != SOURCE_TREE_OK && !selection.valid);
            failure_at = 0; finish();
        }
    }
}
/** Resolved directory capture rejects malformed proof and stale directory identity without publication. */
static void it_rejects_invalid_and_changed_resolved_directory_roots(void) {
    prepare(); SourcePhysicalPath root; SourcePhysicalIssue issue;
    assert(source_physical_resolve(&selection.arena, subtree_path, SOURCE_PHYSICAL_DIRECTORY, false, &root, &issue) == SOURCE_PHYSICAL_OK);
    AcquisitionUnitEntry sentinel = {0}; const AcquisitionUnitEntry *output = &sentinel; const char *failed = NULL;
    SourcePhysicalPath bad = root; bad.metadata.st_ino++;
    assert(acquisition_directory_capture_root(&selection.arena, &selection.files, &bad, &output, &failed) == SOURCE_IO_ERROR);
    assert(output == &sentinel && failed);
    bad = root; bad.present = false;
    assert(acquisition_directory_capture_root(&selection.arena, &selection.files, &bad, &output, &failed) == SOURCE_INVALID_ARGUMENT);
    assert(acquisition_directory_capture_root(&selection.arena, &selection.files, NULL, &output, &failed) == SOURCE_INVALID_ARGUMENT);
    assert(source_tree_selection_read(NULL, &capture, main_path) == SOURCE_TREE_INVALID);
    assert(source_tree_selection_read(&selection, NULL, main_path) == SOURCE_TREE_INVALID);
    assert(source_tree_selection_read(&selection, &capture, NULL) == SOURCE_TREE_INVALID);
    assert(!selection.attempted); finish();
}
/** Physical root membership uses exact slash boundaries, including the filesystem root. */
static void it_matches_physical_root_boundaries_without_basename_or_logical_inference(void) {
    assert(source_tree_inside("/", "/"));
    assert(source_tree_inside("/a/b", "/"));
    assert(source_tree_inside("/a", "/a"));
    assert(source_tree_inside("/a/b", "/a"));
    assert(!source_tree_inside("/ab", "/a"));
    assert(!source_tree_inside("/a", "/a/b"));
    assert(!source_tree_inside("/a b/c", "/a"));
}
/** Raw ignored directories never borrow the selected same-name version's environment. */
static void it_rejects_ignored_versions_and_accepts_selected_scope_without_allocating(void) {
    const char *paths[] = {main_path, "generated/project/dependencies/vendor/pkg/v1.0.0/src", "generated/project"};
    for (size_t index = 0; index < 3; index++) {
        prepare(); assert(source_tree_selection_read(&selection, &capture, paths[index]) == SOURCE_TREE_OK);
        const char *failed = "sentinel"; size_t before = allocation_count;
        assert(source_tree_selection_domain(&selection, &failed) == (index ? SOURCE_TREE_DOMAIN_UNSELECTED_VERSION : SOURCE_TREE_DOMAIN_OK));
        assert(allocation_count == before && (index ? failed != NULL : failed == NULL));
        if (index) { assert(strstr(failed, "/dependencies/vendor/pkg/v1.0.0")); }
        finish();
    }
}
/** Physical analysis scope is the union of explicit roots, each with its own exclusions. */
static void it_honors_exclusions_and_independently_explicit_covering_roots(void) {
    for (unsigned standard = 0; standard < 2; standard++) {
        prepare_scope(true, standard != 0);
        assert(source_tree_selection_read(&selection, &capture, main_path) == SOURCE_TREE_OK);
        const char *failed = NULL;
        assert(source_tree_selection_domain(&selection, &failed) == (standard ? SOURCE_TREE_DOMAIN_OK : SOURCE_TREE_DOMAIN_EXCLUDED));
        if (standard) { assert(selected("//child") && !failed); }
        else { assert(failed && strstr(failed, "/parent+")); }
        finish();
    }
    const char *failed = "sentinel";
    assert(source_tree_selection_domain(NULL, &failed) == SOURCE_TREE_DOMAIN_INVALID && !failed);
    assert(source_tree_selection_domain(NULL, NULL) == SOURCE_TREE_DOMAIN_INVALID);
}
/** Only directories delimit package versions; version-shaped regular files remain opaque members. */
static void it_preserves_version_shaped_opaque_files_without_inventing_package_environments(void) {
    prepare();
    assert(source_tree_selection_read(&selection, &capture, "generated/project/packages") == SOURCE_TREE_OK);
    const char *failed = NULL;
    assert(source_tree_selection_domain(&selection, &failed) == SOURCE_TREE_DOMAIN_OK && !failed);
    bool package_file = false, vendor_file = false;
    for (const AcquisitionUnitEntry *entry = selection.members; entry; entry = entry->next) {
        if (strstr(entry->file.path, "/pkg/v7.8.9")) { package_file = !entry->directory; }
        if (strstr(entry->file.path, "/packages/v4.5.6")) { vendor_file = !entry->directory; }
    }
    assert(package_file && vendor_file && selection.count == 3); finish();
}
/** Run exact physical projection, raw-byte preservation, alias refusal and failure ownership. */
void run(void) {
    it_distinguishes_module_files_from_bare_submodule_directories();
    it_preserves_source_area_and_empty_directory_membership_without_invented_seeds();
    it_refuses_nonmodule_outside_missing_and_aliased_operands();
    it_refuses_changed_original_units_in_file_and_directory_selections();
    it_retires_all_file_directory_and_empty_selection_allocation_failures();
    it_rejects_invalid_and_changed_resolved_directory_roots();
    it_matches_physical_root_boundaries_without_basename_or_logical_inference();
    it_rejects_ignored_versions_and_accepts_selected_scope_without_allocating();
    it_honors_exclusions_and_independently_explicit_covering_roots();
    it_preserves_version_shaped_opaque_files_without_inventing_package_environments();
}
/** Finish only after all projected and original owners have retired. */
int main(void) { run(); return 0; }
