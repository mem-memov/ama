/**
 * @file
 * @brief Verify explicit namespace roots, complete path spelling and absence of cross-family fallback.
 */

/** Both root markers are absolute, every component obeys complete-name rules and no whitespace is trimmed. */
static void it_validates_exact_project_and_standard_paths_with_multiword_components(void) {
    const char *valid[] = {".item", "..item", ".shop tools.item state", "..中国 变量.é value",
        ".a2._ next", ".shop-tools.item-2", ".._item-_value.ключ-2 next_3"};
    for (size_t index = 0; index < sizeof(valid) / sizeof(*valid); index++) { assert(module_path_valid(valid[index])); }
    const char *invalid[] = {"", ".", "..", "...item", "item", ".item.", "..item..child", ". item", "..item ",
        ".shop .item", ".shop. item", ".item  state", ".item 2", ".item\tstate", ".item\xc2\xa0state", ".item+", ".a/child",
        ".-item", ".item-", ".item--name", ".item- name", ".item -name", ".item-.child"};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) { assert(!module_path_valid(invalid[index])); }
    assert(!module_path_valid(NULL));
}

/** Equal component sequences remain separate by declared root kind, regardless of physical name or root order. */
static void it_resolves_explicit_root_families_without_fallback_or_physical_root_names(void) {
    directory("generated/ordinary root");
    directory("generated/库 2");
    file("generated/ordinary root/item state", "@!\n");
    file("generated/ordinary root/item", "@!\n");
    file("generated/ordinary root/only project", "@!\n");
    file("generated/库 2/item state", "@!\n");
    file("generated/库 2/only standard", "@!\n");
    ModuleRoot roots[] = {{"generated/ordinary root",NULL,0,MODULE_ROOT_PROJECT},
        {"generated/库 2",NULL,0,MODULE_ROOT_STANDARD}};
    for (size_t order = 0; order < 2; order++) {
        setup_catalog();
        assert(module_catalog_build(&catalog, roots, 2) == MODULE_OK);
        const ModuleSource *project = NULL, *standard = NULL;
        assert(module_catalog_find(&catalog, ".item state", &project) == MODULE_OK);
        assert(module_catalog_find(&catalog, "..item state", &standard) == MODULE_OK);
        assert(project != standard && strcmp(source_path(project->text), "generated/ordinary root/item state") == 0);
        assert(strcmp(source_path(standard->text), "generated/库 2/item state") == 0);
        const ModuleSource *shorter = NULL;
        assert(module_catalog_find(&catalog, ".item", &shorter) == MODULE_OK && shorter != project);
        assert(strcmp(source_path(shorter->text), "generated/ordinary root/item") == 0);
        const ModuleSource *unchanged = project;
        assert(module_catalog_find(&catalog, ".only standard", &unchanged) == MODULE_MISSING && unchanged == project);
        assert(module_catalog_find(&catalog, "..only project", &unchanged) == MODULE_MISSING && unchanged == project);
        assert(module_catalog_find(&catalog, ".ordinary root.item state", &unchanged) == MODULE_MISSING && unchanged == project);
        assert(module_catalog_find(&catalog, "...item state", &unchanged) == MODULE_INVALID_PATH && unchanged == project);
        destroy_catalog();
        ModuleRoot temporary = roots[0];
        roots[0] = roots[1];
        roots[1] = temporary;
    }
}

/** Unconfigured/unknown root kinds fail before touching filesystem or allocating discovery state. */
static void it_rejects_unconfigured_root_kind_and_missing_root_inventory(void) {
    const ModuleRootKind invalid[] = {(ModuleRootKind)0,(ModuleRootKind)99};
    for (size_t index = 0; index < 2; index++) {
        setup_catalog();
        ModuleRoot root = {.path = "generated/not consulted", .kind = invalid[index]};
        assert(module_catalog_build(&catalog, &root, 1) == MODULE_INVALID_PATH && !catalog.built);
        assert(allocation_count == 0 && catalog.issue.status == MODULE_INVALID_PATH);
        destroy_catalog();
    }
    setup_catalog();
    assert(module_catalog_build(&catalog, NULL, 1) == MODULE_INVALID_PATH && allocation_count == 0);
    destroy_catalog();
}
