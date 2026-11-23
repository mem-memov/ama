/** @file
 * @brief Reopen exact saved staging identities after descriptor ownership expires.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
/** Snapshot actual created identities, not symbolic compiler-image classes. */
static const SourceCatalogImageEntry *const *record_tree(SourceMaterialTree *tree) {
    const SourceCatalogImageEntry *rows = NULL;
    assert(source_material_tree_snapshot(tree, &arena, &rows));
    SourceCatalogImageEntry complete[5];
    complete[0] = after_rows[0];
    for (size_t i = 0; i < tree->count; i++) { complete[i + 1] = rows[i]; }
    struct stat root_metadata;
    assert(fstatat(parent, tree->name, &root_metadata, AT_SYMLINK_NOFOLLOW) == 0);
    assert(rows[0].metadata.st_dev == root_metadata.st_dev && rows[0].metadata.st_ino == root_metadata.st_ino);
    assert(rows[1].metadata.st_size == 70003);
    SourceCatalogImage material;
    SourceImageIssue issue;
    assert(source_catalog_image_prepare(&material, complete, 5, &root, 1, &issue) == SOURCE_IMAGE_OK);
    FILE *stream = tmpfile();
    assert(stream && source_image_document_write(stream, &material));
    long length = ftell(stream);
    assert(length > 0 && fseek(stream, 0, SEEK_SET) == 0);
    unsigned char *bytes = source_arena_allocate(&arena, (size_t)length);
    assert(bytes && fread(bytes, 1, (size_t)length, stream) == (size_t)length && fclose(stream) == 0);
    SourceManager syntax;
    source_manager_init(&syntax, sources.allocator);
    const SourceText *text = NULL;
    assert(source_manager_add(&syntax, "private-record.json", bytes, (size_t)length, &text) == SOURCE_OK);
    assert(source_image_document_read(&recorded, text) == SOURCE_JSON_OK);
    source_manager_destroy(&syntax);
    const SourceCatalogImageEntry **pointers = source_arena_allocate(&arena, tree->count * sizeof(*pointers));
    assert(pointers);
    for (size_t i = 0; i < tree->count; i++) { pointers[i] = &recorded.image.entries[i + 1]; }
    return pointers;
}
/** A fresh recovery owner accepts saved actual classes after all original FDs
 * expire, preserves contents and safely performs repeatable explicit cleanup. */
static void it_reopens_exact_private_ownership_after_the_original_owner_expires(void) {
    begin();
    SourceMaterialTree tree;
    source_material_tree_init(&tree, sources.allocator);
    assert(source_material_tree_prepare(&tree, parent, "copy", forest.roots[0].after_entries,
        forest.roots[0].after_count, (SourceTreeControl){0}) == SOURCE_MATERIAL_TREE_OK);
    const SourceCatalogImageEntry *const *record = record_tree(&tree);
    assert(source_material_tree_destroy(&tree) == 0);
    /* A physical rename preserves saved identity; recovery receives the explicit
     * journal location instead of searching for a similarly named tree. */
    assert(renameat(parent, "copy", parent, "recovered") == 0);
    source_material_tree_init(&tree, sources.allocator);
    assert(source_material_tree_adopt(&tree, parent, "recovered", record, forest.roots[0].after_count, SOURCE_TREE_ADOPT_EXACT,
        (SourceTreeControl){0}) == SOURCE_MATERIAL_TREE_OK && tree.ready);
    assert(source_material_tree_adopt(&tree, parent, "recovered", record, forest.roots[0].after_count, SOURCE_TREE_ADOPT_EXACT,
        (SourceTreeControl){0}) == SOURCE_MATERIAL_TREE_INVALID && tree.ready);
    assert(source_material_tree_verify(&tree) && source_material_tree_cleanup(&tree) && source_material_tree_cleanup(&tree));
    assert(source_material_tree_destroy(&tree) == 0);
    end();
}
/** Reject projected classes, FIFO/same-byte replacements and unknown membership;
 * failed adoption/destruction never removes the actual existing private tree. */
static void it_refuses_unrecorded_classes_and_foreign_private_recovery_material(void) {
    begin();
    SourceMaterialTree original, recovered;
    source_material_tree_init(&original, sources.allocator);
    assert(source_material_tree_prepare(&original, parent, "copy", forest.roots[0].after_entries,
        forest.roots[0].after_count, (SourceTreeControl){0}) == SOURCE_MATERIAL_TREE_OK);
    const SourceCatalogImageEntry *const *record = record_tree(&original);
    source_material_tree_init(&recovered, sources.allocator);
    assert(source_material_tree_adopt(&recovered, parent, "copy", forest.roots[0].after_entries,
        forest.roots[0].after_count, SOURCE_TREE_ADOPT_EXACT, (SourceTreeControl){0}) == SOURCE_MATERIAL_TREE_CHANGED);
    assert(source_material_tree_cleanup(&recovered) && source_material_tree_destroy(&recovered) == 0);
    int copy = openat(parent, "copy", O_RDONLY | O_DIRECTORY);
    assert(copy >= 0 && fchmod(copy, 0700) == 0 && renameat(copy, "z", parent, "saved-z") == 0);
    assert(mkfifoat(copy, "z", 0640) == 0 && fchmod(copy, 0500) == 0);
    source_material_tree_init(&recovered, sources.allocator);
    assert(source_material_tree_adopt(&recovered, parent, "copy", record,
        forest.roots[0].after_count, SOURCE_TREE_ADOPT_EXACT, (SourceTreeControl){0}) == SOURCE_MATERIAL_TREE_CHANGED);
    assert(!source_material_tree_cleanup(&recovered) && source_material_tree_destroy(&recovered) == 0);
    assert(fchmod(copy, 0700) == 0 && unlinkat(copy, "z", 0) == 0);
    int replacement = openat(copy, "z", O_WRONLY | O_CREAT | O_EXCL, 0640);
    assert(replacement >= 0 && close(replacement) == 0 && fchmod(copy, 0500) == 0);
    source_material_tree_init(&recovered, sources.allocator);
    assert(source_material_tree_adopt(&recovered, parent, "copy", record,
        forest.roots[0].after_count, SOURCE_TREE_ADOPT_EXACT, (SourceTreeControl){0}) == SOURCE_MATERIAL_TREE_CHANGED);
    assert(source_material_tree_destroy(&recovered) == 0);
    assert(fchmod(copy, 0700) == 0 && unlinkat(copy, "z", 0) == 0 && renameat(parent, "saved-z", copy, "z") == 0);
    int extra = openat(copy, "foreign", O_WRONLY | O_CREAT | O_EXCL, 0600);
    assert(extra >= 0 && close(extra) == 0 && fchmod(copy, 0500) == 0);
    source_material_tree_init(&recovered, sources.allocator);
    assert(source_material_tree_adopt(&recovered, parent, "copy", record,
        forest.roots[0].after_count, SOURCE_TREE_ADOPT_EXACT, (SourceTreeControl){0}) == SOURCE_MATERIAL_TREE_CHANGED);
    assert(!source_material_tree_cleanup(&recovered) && source_material_tree_destroy(&recovered) == 0);
    assert(fchmod(copy, 0700) == 0 && unlinkat(copy, "foreign", 0) == 0 && fchmod(copy, 0500) == 0 && close(copy) == 0);
    assert(source_material_tree_cleanup(&original) && source_material_tree_destroy(&original) == 0);
    end();
}
/** Every failed reopen/snapshot prefix retains existing material and retires all
 * independently acquired descriptors/allocations without guessed cleanup. */
static void it_retires_every_private_recovery_and_snapshot_failure(void) {
    begin();
    SourceMaterialTree original;
    source_material_tree_init(&original, sources.allocator);
    assert(source_material_tree_prepare(&original, parent, "copy", forest.roots[0].after_entries,
        forest.roots[0].after_count, (SourceTreeControl){0}) == SOURCE_MATERIAL_TREE_OK);
    const SourceCatalogImageEntry *const *record = record_tree(&original);
    size_t calls = 0, allocations = 0, live = live_allocations, open_before = descriptors();
    for (size_t ordinal = 0; !ordinal || ordinal <= calls + allocations; ordinal++) {
        SourceMaterialTree tree;
        source_material_tree_init(&tree, sources.allocator);
        Failure failure = {.fail = ordinal <= calls ? ordinal : 0};
        size_t allocated_before = allocation_count;
        if (ordinal > calls) { failure_at = allocated_before + ordinal - calls; }
        SourceMaterialTreeStatus status = source_material_tree_adopt(&tree, parent, "copy", record,
            forest.roots[0].after_count, SOURCE_TREE_ADOPT_EXACT, (SourceTreeControl){&failure, permit});
        failure_at = 0;
        if (!ordinal) {
            assert(status == SOURCE_MATERIAL_TREE_OK);
            calls = failure.count;
            allocations = allocation_count - allocated_before;
            assert(calls && allocations);
        } else {
            assert(status == (ordinal <= calls ? SOURCE_MATERIAL_TREE_CANCELLED : SOURCE_MATERIAL_TREE_MEMORY) && !tree.ready);
        }
        /* Failed partial adoption leaves the existing tree; this independent
         * original owner is the authority for the final cleanup. */
        assert(source_material_tree_destroy(&tree) == 0 && source_material_tree_verify(&original));
        assert(live_allocations == live && descriptors() == open_before);
    }
    SourceArena failed;
    source_arena_init(&failed, sources.allocator);
    const SourceCatalogImageEntry *output = before_rows;
    failure_at = allocation_count + 1;
    assert(!source_material_tree_snapshot(&original, &failed, &output) && output == before_rows);
    failure_at = 0;
    source_arena_destroy(&failed);
    assert(source_material_tree_cleanup(&original) && source_material_tree_destroy(&original) == 0);
    end();
}

/** An interrupted cleanup can leave only recorded missing entries and the exact
 * owner-access directory mode. A fresh owner resumes without accepting new names. */
static void it_resumes_only_recorded_partial_private_cleanup_transitions(void) {
    begin();
    SourceMaterialTree tree;
    source_material_tree_init(&tree, sources.allocator);
    assert(source_material_tree_prepare(&tree, parent, "copy", forest.roots[0].after_entries,
        forest.roots[0].after_count, (SourceTreeControl){0}) == SOURCE_MATERIAL_TREE_OK);
    const SourceCatalogImageEntry *const *record = record_tree(&tree);
    assert(source_material_tree_destroy(&tree) == 0);
    int copy = openat(parent, "copy", O_RDONLY | O_DIRECTORY);
    assert(copy >= 0 && fchmod(copy, 0700) == 0);
    assert(unlinkat(copy, "a", 0) == 0 && unlinkat(copy, "empty", AT_REMOVEDIR) == 0);
    assert(fchmod(copy, 0740) == 0);
    source_material_tree_init(&tree, sources.allocator);
    assert(source_material_tree_adopt(&tree, parent, "copy", record, forest.roots[0].after_count,
        SOURCE_TREE_ADOPT_CLEANUP, (SourceTreeControl){0}) == SOURCE_MATERIAL_TREE_CHANGED);
    assert(source_material_tree_destroy(&tree) == 0 && fchmod(copy, 0700) == 0);
    int extra = openat(copy, "foreign", O_WRONLY | O_CREAT | O_EXCL, 0600);
    assert(extra >= 0 && close(extra) == 0);
    source_material_tree_init(&tree, sources.allocator);
    assert(source_material_tree_adopt(&tree, parent, "copy", record, forest.roots[0].after_count,
        SOURCE_TREE_ADOPT_CLEANUP, (SourceTreeControl){0}) == SOURCE_MATERIAL_TREE_CHANGED);
    assert(!source_material_tree_cleanup(&tree) && source_material_tree_destroy(&tree) == 0);
    assert(unlinkat(copy, "foreign", 0) == 0 && close(copy) == 0);
    source_material_tree_init(&tree, sources.allocator);
    assert(source_material_tree_adopt(&tree, parent, "copy", record, forest.roots[0].after_count,
        SOURCE_TREE_ADOPT_EXACT, (SourceTreeControl){0}) == SOURCE_MATERIAL_TREE_CHANGED);
    assert(source_material_tree_destroy(&tree) == 0);
    source_material_tree_init(&tree, sources.allocator);
    assert(source_material_tree_adopt(&tree, parent, "copy", record, forest.roots[0].after_count,
        SOURCE_TREE_ADOPT_CLEANUP, (SourceTreeControl){0}) == SOURCE_MATERIAL_TREE_OK && !tree.ready);
    assert(source_material_tree_cleanup(&tree) && source_material_tree_destroy(&tree) == 0);
    /* The entire tree may already have been removed before interruption. */
    source_material_tree_init(&tree, sources.allocator);
    assert(source_material_tree_adopt(&tree, parent, "copy", record, forest.roots[0].after_count,
        SOURCE_TREE_ADOPT_CLEANUP, (SourceTreeControl){0}) == SOURCE_MATERIAL_TREE_OK && !tree.ready);
    assert(source_material_tree_cleanup(&tree) && source_material_tree_destroy(&tree) == 0);
    end();
}
