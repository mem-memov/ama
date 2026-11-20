/**
 * @file
 * @brief Verify selection-to-index handoffs, required package shape and no fallback.
 *
 * The enclosing leaf provides owned physical fixture creation and catalog cleanup.
 * These cases test the actual catalog entry point rather than the selection list alone.
 */

/** A selected local version excludes all external modules, including invalid unused bytes. */
static void it_indexes_only_selected_versions_and_never_fills_local_module_holes(void) {
    prepare_package("generated/precedence");
    prepare_version("generated/precedence", "dependencies", "v1.0.0");
    file("generated/precedence/packages/test/fixture/v1.0.0/src/item", "@!\n");
    file("generated/precedence/dependencies/test/fixture/v1.0.0/src/item", "\xff");
    file("generated/precedence/dependencies/test/fixture/v1.0.0/src/external only", "@!\n");
    setup_catalog();
    ModuleRoot root = {.path = "generated/precedence", .kind = MODULE_ROOT_PROJECT};
    ModuleStatus status = module_catalog_build(&catalog, &root, 1);
    assert(status == MODULE_OK);
    const ModuleSource *source = NULL;
    status = module_catalog_find(&catalog, "/test/fixture/v1.0.0/src/item", &source);
    assert(status == MODULE_OK);
    assert(strcmp(source_path(source->text),
        "generated/precedence/packages/test/fixture/v1.0.0/src/item") == 0);
    const ModuleSource *unchanged = source;
    status = module_catalog_find(&catalog, "/test/fixture/v1.0.0/src/external only", &source);
    assert(status == MODULE_MISSING && source == unchanged);
    assert(catalog.first == unchanged && unchanged->next == NULL);
    destroy_catalog();
}

/** Missing local metadata or areas remain failures even with a complete external copy. */
static void it_reports_missing_selected_package_inputs_without_external_fallback(void) {
    const char *members[] = {"package.ini", "src", "tests"};
    for (size_t index = 0; index < 3; index++) {
        char root_path[80], missing[160];
        int written = snprintf(root_path, sizeof(root_path), "generated/missing-%zu", index);
        assert(written > 0 && (size_t)written < sizeof(root_path));
        prepare_package(root_path);
        prepare_version(root_path, "dependencies", "v1.0.0");
        written = snprintf(missing, sizeof(missing), "%s/packages/test/fixture/v1.0.0/%s",
            root_path, members[index]);
        assert(written > 0 && (size_t)written < sizeof(missing));
        int removed = index == 0 ? unlink(missing) : rmdir(missing);
        assert(removed == 0);
        setup_catalog();
        ModuleRoot root = {.path = root_path, .kind = MODULE_ROOT_PROJECT};
        ModuleStatus status = module_catalog_build(&catalog, &root, 1);
        assert(status == MODULE_MISSING && !catalog.built);
        assert(strcmp(catalog.issue.path, missing) == 0);
        const ModuleSource *source = NULL;
        status = module_catalog_find(&catalog, "/test/fixture/v1.0.0/src/item", &source);
        assert(status == MODULE_INVALID_PATH && source == NULL);
        root_path[0] = 'X';
        assert(catalog.issue.path[0] == 'g');
        destroy_catalog();
    }
}

/** Filtering the chosen version at any level cannot select an ignored copy or root family. */
static void it_does_not_turn_local_exclusions_into_external_or_standard_fallback(void) {
    prepare_package("generated/filtered");
    prepare_version("generated/filtered", "dependencies", "v1.0.0");
    directory("generated/filter-standard");
    file("generated/filtered/packages/test/fixture/v1.0.0/src/item", "@!\n");
    file("generated/filtered/dependencies/test/fixture/v1.0.0/src/item", "@!\n");
    file("generated/filter-standard/item", "@!\n");
    const char *excluded[] = {"packages/test/fixture/v1.0.0",
        "packages/test/fixture/v1.0.0/src", "packages/test/fixture/v1.0.0/src/item"};
    for (size_t index = 0; index < 3; index++) {
        setup_catalog();
        ModuleRoot roots[] = {
            {.path = "generated/filtered", .excluded = &excluded[index], .excluded_count = 1,
                .kind = MODULE_ROOT_PROJECT},
            {.path = "generated/filter-standard", .kind = MODULE_ROOT_STANDARD}
        };
        ModuleStatus status = module_catalog_build(&catalog, roots, 2);
        assert(status == MODULE_OK);
        const ModuleSource *source = NULL;
        status = module_catalog_find(&catalog, "//item", &source);
        assert(status == MODULE_OK);
        const ModuleSource *unchanged = source;
        status = module_catalog_find(&catalog, "/test/fixture/v1.0.0/src/item", &source);
        assert(status == MODULE_MISSING && source == unchanged);
        destroy_catalog();
    }
}

/** Dependency-only layouts expose both explicit versions instead of choosing a release. */
static void it_indexes_dependency_only_projects_and_keeps_versions_distinct(void) {
    prepare_version("generated/dependency-only", "dependencies", "v1.0.0");
    prepare_version("generated/dependency-only", "dependencies", "v1.0.0-speed");
    file("generated/dependency-only/dependencies/test/fixture/v1.0.0/src/item", "@!\n");
    file("generated/dependency-only/dependencies/test/fixture/v1.0.0-speed/src/item", "@!\n");
    file("generated/dependency-only/dependencies/test/fixture/v1.0.0-speed/tests/verify item", "@!\n");
    setup_catalog();
    ModuleRoot root = {.path = "generated/dependency-only", .kind = MODULE_ROOT_PROJECT};
    ModuleStatus status = module_catalog_build(&catalog, &root, 1);
    assert(status == MODULE_OK);
    const ModuleSource *ordinary = NULL, *variant = NULL, *test = NULL;
    status = module_catalog_find(&catalog, "/test/fixture/v1.0.0/src/item", &ordinary);
    assert(status == MODULE_OK);
    status = module_catalog_find(&catalog, "/test/fixture/v1.0.0-speed/src/item", &variant);
    assert(status == MODULE_OK && ordinary != variant);
    status = module_catalog_find(&catalog, "/test/fixture/v1.0.0-speed/tests/verify item", &test);
    assert(status == MODULE_OK && test != variant);
    destroy_catalog();
}
