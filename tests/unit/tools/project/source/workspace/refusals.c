/** @file
 * @brief Preserve source inputs and foreign private objects on rejected workspace operations.
 * @see [T287](../../../../../../docs/implementation/source-operations.md#t287-entry-review-after-accepted-t286)
 */
/** Invalid/protected parents never authorize private creation within sources. */
static void it_refuses_unavailable_or_protected_workspace_inputs_without_source_mutation(void) {
    begin();
    const char *protected[] = {captured.roots[0].path};
    SourceWorkspace workspace;
    source_workspace_init(&workspace, sources.allocator);
    assert(source_workspace_verify(&workspace) == SOURCE_WORKSPACE_INVALID);
    assert(source_workspace_open(&workspace, captured.roots[0].path, protected, 1, (SourceTreeControl){0}) == SOURCE_WORKSPACE_BOUNDARY);
    assert(source_workspace_cleanup(&workspace) == SOURCE_WORKSPACE_OK && source_workspace_destroy(&workspace) == 0);
    const char *parents[] = {"generated/absent", "generated/source/main", NULL};
    for (size_t i = 0; i < 3; i++) {
        source_workspace_init(&workspace, sources.allocator);
        SourceWorkspaceStatus status = source_workspace_open(&workspace, parents[i], protected, 1, (SourceTreeControl){0});
        assert(status == (i == 2 ? SOURCE_WORKSPACE_INVALID : SOURCE_WORKSPACE_PHYSICAL));
        assert(source_workspace_cleanup(&workspace) == SOURCE_WORKSPACE_OK && source_workspace_destroy(&workspace) == 0);
    }
    source_workspace_init(&workspace, sources.allocator);
    assert(source_workspace_open(&workspace, temporary_parent, protected, SIZE_MAX, (SourceTreeControl){0}) == SOURCE_WORKSPACE_LIMIT);
    assert(source_workspace_cleanup(&workspace) == SOURCE_WORKSPACE_OK && source_workspace_destroy(&workspace) == 0);
    end();
}
/** Unknown membership, changed request bytes and replaced root names all block
 * deletion. Restoring test-owned interference permits safe cleanup retries. */
static void it_preserves_foreign_workspace_members_bytes_and_root_replacements(void) {
    begin(); SourceWorkspace workspace;
    source_workspace_init(&workspace, sources.allocator);
    assert(source_workspace_open(&workspace, temporary_parent, NULL, 0, (SourceTreeControl){0}) == SOURCE_WORKSPACE_OK);
    assert(source_workspace_snapshot(&workspace, SOURCE_WORKSPACE_ORIGINAL, &captured.view) == SOURCE_WORKSPACE_OK);
    int descriptor = openat(workspace.directory, "foreign", O_WRONLY | O_CREAT | O_EXCL, 0600); assert(descriptor >= 0);
    assert(write(descriptor, "keep", 4) == 4 && close(descriptor) == 0);
    assert(source_workspace_verify(&workspace) == SOURCE_WORKSPACE_CHANGED);
    assert(source_workspace_cleanup(&workspace) == SOURCE_WORKSPACE_CHANGED && members(workspace.path) == 2);
    assert(unlinkat(workspace.directory, "foreign", 0) == 0);
    assert(source_workspace_cleanup(&workspace) == SOURCE_WORKSPACE_OK && workspace.status == SOURCE_WORKSPACE_CHANGED);
    assert(source_workspace_destroy(&workspace) == 0);
    source_workspace_init(&workspace, sources.allocator);
    assert(source_workspace_open(&workspace, temporary_parent, NULL, 0, (SourceTreeControl){0}) == SOURCE_WORKSPACE_OK);
    assert(source_workspace_snapshot(&workspace, SOURCE_WORKSPACE_ORIGINAL, &captured.view) == SOURCE_WORKSPACE_OK);
    SourceWorkspaceFile *file = &workspace.files[SOURCE_WORKSPACE_ORIGINAL];
    descriptor = open(file->entry.path, O_WRONLY); assert(descriptor >= 0);
    unsigned char byte = source_bytes(file->entry.content)[0], changed = (unsigned char)(byte ^ 1);
    assert(pwrite(descriptor, &changed, 1, 0) == 1);
    assert(source_workspace_verify(&workspace) == SOURCE_WORKSPACE_CHANGED && source_workspace_cleanup(&workspace) == SOURCE_WORKSPACE_CHANGED);
    assert(pwrite(descriptor, &byte, 1, 0) == 1 && close(descriptor) == 0);
    assert(source_workspace_cleanup(&workspace) == SOURCE_WORKSPACE_OK && source_workspace_destroy(&workspace) == 0);
    source_workspace_init(&workspace, sources.allocator);
    assert(source_workspace_open(&workspace, temporary_parent, NULL, 0, (SourceTreeControl){0}) == SOURCE_WORKSPACE_OK);
    assert(source_workspace_snapshot(&workspace, SOURCE_WORKSPACE_ORIGINAL, &captured.view) == SOURCE_WORKSPACE_OK);
    assert(renameat(workspace.parent, workspace.name, workspace.parent, "saved") == 0);
    assert(mkdirat(workspace.parent, workspace.name, 0700) == 0);
    assert(source_workspace_verify(&workspace) == SOURCE_WORKSPACE_CHANGED && source_workspace_cleanup(&workspace) == SOURCE_WORKSPACE_CHANGED);
    assert(unlinkat(workspace.parent, workspace.name, AT_REMOVEDIR) == 0);
    assert(renameat(workspace.parent, "saved", workspace.parent, workspace.name) == 0);
    assert(source_workspace_cleanup(&workspace) == SOURCE_WORKSPACE_OK && source_workspace_destroy(&workspace) == 0);
    end();
}
