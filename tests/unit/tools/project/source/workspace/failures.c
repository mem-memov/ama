/** @file
 * @brief Exhaust measured allocator and controlled private-write failure prefixes with resource accounting.
 * @see [T287](../../../../../../docs/implementation/source-operations.md#t287-entry-review-after-accepted-t286)
 */
/** Fail one explicit private-tree operation boundary; cleanup is uncancellable. */
typedef struct { size_t count, fail; } WorkspaceControl;
/** Count actual boundaries without inferring success from callback counts. */
static bool workspace_permit(void *context, SourceTreeOperation operation, const char *path) {
    (void)operation; assert(path); WorkspaceControl *control = context;
    control->count++; return control->count != control->fail;
}
/** Use genuine serialization and both physical file owners until the first failure. */
static SourceWorkspaceStatus prepare_requests(SourceWorkspace *workspace, WorkspaceControl *control) {
    SourceWorkspaceStatus status = source_workspace_open(workspace, temporary_parent, NULL, 0,
        (SourceTreeControl){control, workspace_permit});
    if (status == SOURCE_WORKSPACE_OK) { status = source_workspace_snapshot(workspace, SOURCE_WORKSPACE_ORIGINAL, &captured.view); }
    if (status == SOURCE_WORKSPACE_OK) { status = source_workspace_snapshot(workspace, SOURCE_WORKSPACE_FINAL, &captured.view); }
    return status;
}
/** Every measured custom allocation and tree boundary must either complete or
 * retain a cleanable owned prefix, with primary diagnostics and no descriptor leak. */
static void it_retires_workspace_allocation_and_cancelled_prefixes_with_primary_issues_intact(void) {
    begin(); size_t baseline = live_allocations, opened = descriptors();
    SourceWorkspace workspace; WorkspaceControl control = {0};
    source_workspace_init(&workspace, sources.allocator); allocation_count = 0;
    assert(prepare_requests(&workspace, &control) == SOURCE_WORKSPACE_OK);
    size_t allocations = allocation_count, boundaries = control.count;
    assert(allocations && boundaries);
    assert(source_workspace_cleanup(&workspace) == SOURCE_WORKSPACE_OK && source_workspace_destroy(&workspace) == 0);
    assert(live_allocations == baseline && descriptors() == opened);
    for (size_t i = 1; i <= allocations; i++) {
        source_workspace_init(&workspace, sources.allocator); control = (WorkspaceControl){0};
        allocation_count = 0; failure_at = i;
        SourceWorkspaceStatus status = prepare_requests(&workspace, &control);
        failure_at = 0;
        assert(status != SOURCE_WORKSPACE_OK && !workspace.ready);
        SourceWorkspaceIssue primary = workspace.issue;
        assert(source_workspace_cleanup(&workspace) == SOURCE_WORKSPACE_OK && workspace.status == status
            && workspace.issue.path == primary.path && workspace.issue.system_error == primary.system_error);
        assert(source_workspace_destroy(&workspace) == 0 && live_allocations == baseline && descriptors() == opened);
        assert(members(temporary_parent) == 0);
    }
    for (size_t i = 1; i <= boundaries; i++) {
        source_workspace_init(&workspace, sources.allocator); control = (WorkspaceControl){.fail = i};
        SourceWorkspaceStatus status = prepare_requests(&workspace, &control);
        assert(status == SOURCE_WORKSPACE_TREE && !workspace.ready);
        SourceWorkspaceIssue primary = workspace.issue;
        assert(source_workspace_cleanup(&workspace) == SOURCE_WORKSPACE_OK && workspace.status == status
            && workspace.issue.path == primary.path && workspace.issue.tree_status == primary.tree_status);
        assert(source_workspace_destroy(&workspace) == 0 && live_allocations == baseline && descriptors() == opened);
        assert(members(temporary_parent) == 0);
    }
    end();
}
