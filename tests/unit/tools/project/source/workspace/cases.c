/** @file
 * @brief Verify actual two-request workspace composition, immutable bytes and precreation protection/collisions.
 * @see [T287 entry review](../../../../../../docs/implementation/source-operations.md#t287-entry-review-after-accepted-t286)
 */
/** Root readiness is honestly invalidated when independent child ownership is
 * added, while the workspace proves the complete joined private membership. */
static void it_owns_actual_original_and_final_requests_outside_sources_and_cleans_them_safely(void) {
    begin(); size_t baseline = live_allocations, opened = descriptors();
    SourceWorkspace workspace; source_workspace_init(&workspace, sources.allocator);
    const char *protected[] = {captured.roots[0].path};
    assert(source_workspace_open(&workspace, temporary_parent, protected, 1, (SourceTreeControl){0}) == SOURCE_WORKSPACE_OK
        && workspace.ready && workspace.root.ready && members(workspace.path) == 0);
    assert(source_workspace_snapshot(&workspace, SOURCE_WORKSPACE_ORIGINAL, &captured.view) == SOURCE_WORKSPACE_OK
        && workspace.ready && !workspace.root.ready && workspace.files[SOURCE_WORKSPACE_ORIGINAL].tree.ready);
    request_bytes(&workspace, SOURCE_WORKSPACE_ORIGINAL);
    assert(source_workspace_snapshot(&workspace, SOURCE_WORKSPACE_FINAL, &captured.view) == SOURCE_WORKSPACE_OK && members(workspace.path) == 2);
    request_bytes(&workspace, SOURCE_WORKSPACE_FINAL);
    assert(source_workspace_verify(&workspace) == SOURCE_WORKSPACE_OK);
    size_t calls = allocation_count;
    assert(source_workspace_snapshot(&workspace, SOURCE_WORKSPACE_ORIGINAL, &captured.view) == SOURCE_WORKSPACE_INVALID
        && workspace.ready && allocation_count == calls);
    assert(source_workspace_open(&workspace, temporary_parent, protected, 1, (SourceTreeControl){0}) == SOURCE_WORKSPACE_INVALID && workspace.ready);
    const char *path = source_arena_copy(&fixture_arena, workspace.path); assert(path);
    assert(source_workspace_cleanup(&workspace) == SOURCE_WORKSPACE_OK && source_workspace_cleanup(&workspace) == SOURCE_WORKSPACE_OK);
    assert(access(path, F_OK) != 0 && errno == ENOENT && source_workspace_verify(&workspace) == SOURCE_WORKSPACE_INVALID);
    assert(source_workspace_destroy(&workspace) == 0 && live_allocations == baseline + 1 && descriptors() == opened);
    end();
}
/** The first absent protected name is skipped before mkdir, and existing foreign
 * names remain untouched even when they resemble this process's workspace names. */
static void it_skips_protected_and_existing_private_names_before_any_creation(void) {
    begin(); char names[2][SOURCE_WORKSPACE_NAME_CAPACITY];
    for (size_t i = 0; i < 2; i++) {
        int count = snprintf(names[i], sizeof(names[i]), ".ama-source-%ju-%zu", (uintmax_t)getpid(), i);
        assert(count > 0 && (size_t)count < sizeof(names[i]));
    }
    const char *forbidden = source_arena_join(&fixture_arena, temporary_parent, "/", names[0]);
    const char *foreign = source_arena_join(&fixture_arena, temporary_parent, "/", names[1]); assert(forbidden && foreign);
    FILE *kept = fopen(foreign, "wb"); assert(kept && fwrite("keep", 1, 4, kept) == 4 && fclose(kept) == 0);
    const char *protected[] = {captured.roots[0].path, forbidden};
    SourceWorkspace workspace; source_workspace_init(&workspace, sources.allocator);
    assert(source_workspace_open(&workspace, temporary_parent, protected, 2, (SourceTreeControl){0}) == SOURCE_WORKSPACE_OK);
    assert(strcmp(workspace.path, forbidden) && strcmp(workspace.path, foreign));
    assert(access(forbidden, F_OK) != 0 && errno == ENOENT && members(temporary_parent) == 2);
    assert(source_workspace_cleanup(&workspace) == SOURCE_WORKSPACE_OK && source_workspace_destroy(&workspace) == 0);
    assert(members(temporary_parent) == 1 && unlink(foreign) == 0); end();
}
