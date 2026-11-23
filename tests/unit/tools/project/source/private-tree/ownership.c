/** @file
 * @brief Query genuine created and partial inode ownership independently of readiness and content verification.
 * @see [T287 entry review](../../../../../../docs/implementation/source-operations.md#t287-entry-review-after-accepted-t286)
 */
/** A partial created file can be owned before ready, while a same-name collision
 * remains foreign; cleanup and removed epochs never recreate old authority. */
static void it_matches_only_actual_identified_unremoved_private_inode_ownership(void) {
    begin(); SourceMaterialTree tree; source_material_tree_init(&tree, sources.allocator);
    struct stat actual = {0};
    const char *key = forest.roots[0].after_entries[1]->path;
    assert(!source_material_tree_owns(&tree, key, &actual));
    Failure failure = {.fail = 2};
    assert(source_material_tree_prepare(&tree, parent, "partial", forest.roots[0].after_entries + 1, 1,
        (SourceTreeControl){&failure, permit}) == SOURCE_MATERIAL_TREE_CANCELLED && !tree.ready);
    assert(fstatat(parent, "partial", &actual, AT_SYMLINK_NOFOLLOW) == 0 && !actual.st_size);
    size_t calls = allocation_count;
    assert(source_material_tree_owns(&tree, key, &actual) && allocation_count == calls);
    struct stat other = actual; other.st_ino++;
    assert(!source_material_tree_owns(&tree, key, &other));
    other = actual; other.st_dev++;
    assert(!source_material_tree_owns(&tree, key, &other));
    other = actual; other.st_mode ^= S_IXUSR;
    assert(!source_material_tree_owns(&tree, key, &other));
    assert(!source_material_tree_owns(&tree, "/unknown", &actual));
    assert(!source_material_tree_owns(NULL, key, &actual) && !source_material_tree_owns(&tree, NULL, &actual)
        && !source_material_tree_owns(&tree, key, NULL));
    assert(source_material_tree_cleanup(&tree) && !source_material_tree_owns(&tree, key, &actual));
    assert(source_material_tree_destroy(&tree) == 0 && !source_material_tree_owns(&tree, key, &actual));
    int foreign = openat(parent, "foreign", O_CREAT | O_EXCL | O_WRONLY, 0600); assert(foreign >= 0 && close(foreign) == 0);
    assert(fstatat(parent, "foreign", &actual, AT_SYMLINK_NOFOLLOW) == 0);
    source_material_tree_init(&tree, sources.allocator);
    assert(source_material_tree_prepare(&tree, parent, "foreign", forest.roots[0].after_entries + 1, 1,
        (SourceTreeControl){0}) == SOURCE_MATERIAL_TREE_IO);
    assert(!source_material_tree_owns(&tree, key, &actual));
    assert(source_material_tree_cleanup(&tree) && source_material_tree_destroy(&tree) == 0);
    assert(unlinkat(parent, "foreign", 0) == 0); end();
}
