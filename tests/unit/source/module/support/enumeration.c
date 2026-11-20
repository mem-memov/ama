/**
 * @file
 * @brief Verify sorted discovery independently of creation order and exact owned-output exclusion boundaries.
 */

/** Reverse insertion plus FIFO directory scanning produces one deterministic reverse-publication sequence. */
static void it_enumerates_the_same_catalog_independently_of_file_creation_order(void) {
    prepare_package("generated/order first");
    prepare_package("generated/order second");
    const char *roots[] = {"generated/order first", "generated/order second"};
    const char *names[] = {"zeta", "alpha", "group/second", "group/first"};
    for (size_t order = 0; order < 2; order++) {
        char path[128];
        int length = snprintf(path, sizeof(path), "%s/packages/test/fixture/v1.0.0/src/group", roots[order]);
        assert(length > 0 && (size_t)length < sizeof(path));
        directory(path);
        for (size_t index = 0; index < 4; index++) {
            size_t selected = order == 0 ? index : 3 - index;
            length = snprintf(path, sizeof(path), "%s/packages/test/fixture/v1.0.0/src/%s", roots[order], names[selected]);
            assert(length > 0 && (size_t)length < sizeof(path));
            file(path, "@C\n");
        }
        setup_catalog();
        ModuleRoot root = {roots[order], NULL, 0, MODULE_ROOT_PROJECT};
        assert(module_catalog_build(&catalog, &root, 1) == MODULE_OK);
        const char *expected[] = {"/test/fixture/v1.0.0/src/group/second", "/test/fixture/v1.0.0/src/group/first", "/test/fixture/v1.0.0/src/zeta", "/test/fixture/v1.0.0/src/alpha"};
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
    prepare_package("generated/owned");
    directory("generated/owned/packages/test/fixture/v1.0.0/src/output");
    directory("generated/owned/packages/test/fixture/v1.0.0/src/output/nested");
    directory("generated/owned/packages/test/fixture/v1.0.0/src/output next");
    directory("generated/owned/packages/test/fixture/v1.0.0/src/other");
    directory("generated/owned/packages/test/fixture/v1.0.0/src/other/output");
    file("generated/owned/packages/test/fixture/v1.0.0/src/output/nested/broken", "\xff");
    file("generated/owned/packages/test/fixture/v1.0.0/src/output next/item", "@C\n");
    file("generated/owned/packages/test/fixture/v1.0.0/src/other/output/item", "@C\n");
    file("generated/owned/packages/test/fixture/v1.0.0/src/exact", "\xff");
    file("generated/owned/packages/test/fixture/v1.0.0/src/exact next", "@C\n");
    const char *excluded[] = {"packages/test/fixture/v1.0.0/src/output",
        "packages/test/fixture/v1.0.0/src/exact"};
    setup_catalog();
    ModuleRoot root = {"generated/owned", excluded, 2, MODULE_ROOT_PROJECT};
    assert(module_catalog_build(&catalog, &root, 1) == MODULE_OK);
    const char *present[] = {"/test/fixture/v1.0.0/src/output next/item", "/test/fixture/v1.0.0/src/other/output/item", "/test/fixture/v1.0.0/src/exact next"};
    const ModuleSource *source = NULL;
    for (size_t index = 0; index < 3; index++) {
        assert(module_catalog_find(&catalog, present[index], &source) == MODULE_OK);
    }
    const ModuleSource *unchanged = source;
    assert(module_catalog_find(&catalog, "/test/fixture/v1.0.0/src/output/nested/broken", &source) == MODULE_MISSING);
    assert(module_catalog_find(&catalog, "/test/fixture/v1.0.0/src/exact", &source) == MODULE_MISSING);
    assert(source == unchanged);
    destroy_catalog();
}

/** A physical root with CR cannot safely reach GCC __FILE__; retain the original path in an explicit unsupported-path diagnostic. */
static void it_rejects_cr_roots_unsupported_by_the_verified_c_target(void) {
    directory("generated/root\rname");
    file("generated/root\rname/item", "@C\n");
    setup_catalog();
    char path[] = "generated/root\rname";
    ModuleRoot root = {path, NULL, 0, MODULE_ROOT_PROJECT};
    ModuleStatus status = module_catalog_build(&catalog, &root, 1);
    assert(status == MODULE_UNSUPPORTED_PATH && !catalog.built);
    path[0] = 'X';
    assert(strcmp(catalog.issue.path, "generated/root\rname") == 0);
    destroy_catalog();
}
