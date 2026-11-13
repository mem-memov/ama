/**
 * @file
 * @brief Verify sorted discovery independently of creation order and exact owned-output exclusion boundaries.
 */

/** Reverse insertion plus FIFO directory scanning produces one deterministic reverse-publication sequence. */
static void it_enumerates_the_same_catalog_independently_of_file_creation_order(void) {
    directory("generated/order first");
    directory("generated/order second");
    const char *roots[] = {"generated/order first", "generated/order second"};
    const char *names[] = {"zeta", "alpha", "group/second", "group/first"};
    for (size_t order = 0; order < 2; order++) {
        char path[128];
        int length = snprintf(path, sizeof(path), "%s/group", roots[order]);
        assert(length > 0 && (size_t)length < sizeof(path));
        directory(path);
        for (size_t index = 0; index < 4; index++) {
            size_t selected = order == 0 ? index : 3 - index;
            length = snprintf(path, sizeof(path), "%s/%s", roots[order], names[selected]);
            assert(length > 0 && (size_t)length < sizeof(path));
            file(path, "@C\n");
        }
        setup_catalog();
        ModuleRoot root = {roots[order], NULL, 0, MODULE_ROOT_PROJECT};
        assert(module_catalog_build(&catalog, &root, 1) == MODULE_OK);
        const char *expected[] = {".group.second", ".group.first", ".zeta", ".alpha"};
        const ModuleSource *source = catalog.first;
        for (size_t index = 0; index < 4; index++) {
            assert(source != NULL && strcmp(source->logical_path, expected[index]) == 0);
            source = source->next;
        }
        assert(source == NULL);
        destroy_catalog();
    }
}

/** Excluded output subtrees are skipped before UTF-8/alias checks; similarly named source directories remain visible. */
static void it_excludes_only_declared_owned_paths_and_slash_delimited_descendants(void) {
    directory("generated/owned");
    directory("generated/owned/output");
    directory("generated/owned/output/nested");
    directory("generated/owned/output next");
    directory("generated/owned/other");
    directory("generated/owned/other/output");
    file("generated/owned/output/nested/broken", "\xff");
    file("generated/owned/output next/item", "@C\n");
    file("generated/owned/other/output/item", "@C\n");
    file("generated/owned/exact", "\xff");
    file("generated/owned/exact next", "@C\n");
    const char *excluded[] = {"output", "exact"};
    setup_catalog();
    ModuleRoot root = {"generated/owned", excluded, 2, MODULE_ROOT_PROJECT};
    assert(module_catalog_build(&catalog, &root, 1) == MODULE_OK);
    const char *present[] = {".output next.item", ".other.output.item", ".exact next"};
    const ModuleSource *source = NULL;
    for (size_t index = 0; index < 3; index++) {
        assert(module_catalog_find(&catalog, present[index], &source) == MODULE_OK);
    }
    const ModuleSource *unchanged = source;
    assert(module_catalog_find(&catalog, ".output.nested.broken", &source) == MODULE_MISSING);
    assert(module_catalog_find(&catalog, ".exact", &source) == MODULE_MISSING);
    assert(source == unchanged);
    destroy_catalog();
}
