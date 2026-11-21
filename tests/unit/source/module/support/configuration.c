/** @file
 * @brief Verify exact selected configuration ownership and source-only snapshot attachment.
 * The catalog leaf supplies counted allocation, physical tree and source fixtures.
 */

/** Selected and empty versions retain independent immutable sources after disk/root changes. */
static void it_retains_selected_version_configuration_and_exact_module_ownership(void) {
    prepare_version("generated/context", "packages", "v1.0.0");
    prepare_version("generated/context", "packages", "v1.0.0-fast");
    prepare_version("generated/context", "dependencies", "v1.0.0");
    const char *configuration = "generated/context/packages/test/fixture/v1.0.0/package.ini";
    file(configuration, "[aliases]\r\n~local = ./src");
    file("generated/context/dependencies/test/fixture/v1.0.0/package.ini", "\xff");
    file("generated/context/packages/test/fixture/v1.0.0/src/value", "@!\n");
    file("generated/context/packages/test/fixture/v1.0.0/tests/check", "@!\n");
    char root_path[] = "generated/context";
    ModuleRoot root = {.path = root_path, .kind = MODULE_ROOT_PROJECT};
    setup_catalog();
    assert(module_catalog_build(&catalog, &root, 1) == MODULE_OK);
    root_path[0] = 'X';
    const ModuleSource *source = NULL, *test = NULL;
    assert(module_catalog_find(&catalog, "/test/fixture/v1.0.0/src/value", &source) == MODULE_OK);
    assert(module_catalog_find(&catalog, "/test/fixture/v1.0.0/tests/check", &test) == MODULE_OK);
    assert(source->version != NULL && source->version == test->version);
    const ModuleVersion *version = source->version;
    assert(strcmp(version->logical_path, "/test/fixture/v1.0.0") == 0);
    assert(strcmp(version->physical_path, "generated/context/packages/test/fixture/v1.0.0") == 0);
    assert(strcmp(source_path(version->configuration), configuration) == 0);
    file(configuration, "changed on disk");
    assert(source_length(version->configuration) == strlen("[aliases]\r\n~local = ./src"));
    assert(memcmp(source_bytes(version->configuration), "[aliases]\r\n~local = ./src",
        source_length(version->configuration)) == 0);
    size_t count = 0;
    bool empty_variant = false;
    for (const ModuleVersion *entry = catalog.versions; entry != NULL; entry = entry->next) {
        count++;
        if (strcmp(entry->logical_path, "/test/fixture/v1.0.0-fast") == 0) {
            empty_variant = true;
            assert(entry != version && source_length(entry->configuration) == 0);
        }
    }
    assert(count == 2 && empty_variant);
    destroy_catalog();
}

/** Snapshot ownership follows complete version boundaries and never probes physical paths. */
static void it_attaches_copied_configuration_to_exact_snapshot_versions(void) {
    setup_catalog();
    const SourceText *text = NULL, *configuration = NULL;
    assert(source_manager_add(&sources, "absent/value", (const unsigned char *)"@!\n", 3, &text) == SOURCE_OK);
    const unsigned char bytes[] = {';', 'x', 0, 'y', '\r', '\n'};
    assert(source_manager_add(&sources, "absent/package.ini", bytes, sizeof(bytes), &configuration) == SOURCE_OK);
    ModuleSnapshotEntry entries[] = {
        {"/test/fixture/v1.0.0/src/value", text, 1, 1},
        {"/test/fixture/v1.0.0/tests/check", text, 1, 2},
        {"/test/fixture/v1.0.0-fast/src/value", text, 1, 3},
        {"//value", text, 1, 4}
    };
    assert(module_catalog_snapshot(&catalog, entries, 4) == MODULE_OK);
    char logical[] = "/test/fixture/v1.0.0";
    char physical[] = "absent/version";
    assert(module_catalog_snapshot_configuration(&catalog, logical, physical, configuration) == MODULE_OK);
    logical[1] = 'X';
    physical[0] = 'X';
    source_manager_destroy(&sources);
    const ModuleVersion *version = catalog.versions;
    assert(version != NULL && version->next == NULL);
    assert(strcmp(version->logical_path, "/test/fixture/v1.0.0") == 0);
    assert(strcmp(version->physical_path, "absent/version") == 0);
    assert(strcmp(source_path(version->configuration), "absent/package.ini") == 0);
    assert(source_length(version->configuration) == sizeof(bytes));
    assert(memcmp(source_bytes(version->configuration), bytes, sizeof(bytes)) == 0);
    for (const ModuleSource *source = catalog.first; source != NULL; source = source->next) {
        bool matching = strcmp(source->logical_path, entries[0].logical_path) == 0
            || strcmp(source->logical_path, entries[1].logical_path) == 0;
        assert(source->version == (matching ? version : NULL));
    }
    assert(module_catalog_snapshot_configuration(&catalog, "/test/fixture/v1.0.0",
        "another directory", version->configuration) == MODULE_AMBIGUOUS);
    assert(catalog.versions == version && version->next == NULL);
    assert(strcmp(catalog.issue.related_path, "absent/package.ini") == 0);
    destroy_catalog();
}

/** Every measured copy/publication failure leaves module context links and prior owners intact. */
static void it_unwinds_every_configuration_snapshot_allocation_failure(void) {
    setup_catalog();
    const SourceText *text = NULL, *configuration = NULL;
    assert(source_manager_add(&sources, "absent/value", (const unsigned char *)"@!\n", 3, &text) == SOURCE_OK);
    assert(source_manager_add(&sources, "absent/package.ini", NULL, 0, &configuration) == SOURCE_OK);
    ModuleSnapshotEntry entry = {"/test/fixture/v1.0.0/src/value", text, 1, 1};
    size_t baseline = live_allocations;
    assert(module_catalog_snapshot(&catalog, &entry, 1) == MODULE_OK);
    allocation_count = 0;
    assert(module_catalog_snapshot_configuration(&catalog, "/test/fixture/v1.0.0", "absent", configuration) == MODULE_OK);
    size_t total = allocation_count;
    module_catalog_destroy(&catalog);
    assert(live_allocations == baseline);
    for (size_t failure = 1; failure <= total; failure++) {
        module_catalog_init(&catalog, sources.allocator);
        failure_at = 0;
        assert(module_catalog_snapshot(&catalog, &entry, 1) == MODULE_OK);
        allocation_count = 0;
        failure_at = failure;
        assert(module_catalog_snapshot_configuration(&catalog, "/test/fixture/v1.0.0", "absent", configuration)
            == MODULE_OUT_OF_MEMORY);
        assert(catalog.versions == NULL && catalog.first->version == NULL && catalog.built);
        assert(source_length(configuration) == 0 && source_bytes(text)[0] == '@');
        module_catalog_destroy(&catalog);
        assert(live_allocations == baseline);
    }
    failure_at = 0;
    module_catalog_init(&catalog, sources.allocator);
    destroy_catalog();
}

/** Reject malformed endpoints and retain encoding diagnostics without altering attached contexts. */
static void it_refuses_invalid_configuration_snapshot_contexts_and_retains_utf8_sites(void) {
    setup_catalog();
    const SourceText *configuration = NULL;
    assert(source_manager_add(&sources, "absent/package.ini", (const unsigned char *)";\n\xff", 3,
        &configuration) == SOURCE_OK);
    assert(module_catalog_snapshot_configuration(&catalog, "/test/fixture/v1.0.0", "absent", configuration)
        == MODULE_INVALID_PATH);
    assert(module_catalog_snapshot(&catalog, NULL, 0) == MODULE_OK);
    const char *invalid[] = {NULL, "//value", "/test/fixture", "/test/fixture/v1.0.0/src/value", "/test/fixture/v1.0.0/"};
    for (size_t index = 0; index < sizeof(invalid)/sizeof(invalid[0]); index++) {
        assert(module_catalog_snapshot_configuration(&catalog, invalid[index], "absent", configuration)
            == MODULE_INVALID_PATH);
        assert(catalog.versions == NULL);
    }
    assert(module_catalog_snapshot_configuration(NULL, "/test/fixture/v1.0.0", "absent", configuration) == MODULE_INVALID_PATH);
    assert(module_catalog_snapshot_configuration(&catalog, "/test/fixture/v1.0.0", NULL, configuration) == MODULE_INVALID_PATH);
    assert(module_catalog_snapshot_configuration(&catalog, "/test/fixture/v1.0.0", "", configuration) == MODULE_INVALID_PATH);
    assert(module_catalog_snapshot_configuration(&catalog, "/test/fixture/v1.0.0", "absent", NULL) == MODULE_INVALID_PATH);
    assert(module_catalog_snapshot_configuration(&catalog, "/test/fixture/v1.0.0", "absent", configuration) == MODULE_INVALID_UTF8);
    assert(catalog.versions == NULL && catalog.issue.offset == 2);
    source_manager_destroy(&sources);
    assert(source_length(catalog.issue.source) == 3);
    assert(source_bytes(catalog.issue.source)[2] == 255);
    assert(strcmp(catalog.issue.path, "absent/package.ini") == 0);
    destroy_catalog();
}

/** Actual selected malformed configuration reports its exact owned encoding location. */
static void it_reports_selected_configuration_utf8_without_publishing_a_catalog(void) {
    prepare_package("generated/context-invalid");
    file("generated/context-invalid/packages/test/fixture/v1.0.0/package.ini", ";\n\xff");
    ModuleRoot root = {.path = "generated/context-invalid", .kind = MODULE_ROOT_PROJECT};
    setup_catalog();
    assert(module_catalog_build(&catalog, &root, 1) == MODULE_INVALID_UTF8);
    assert(!catalog.built && catalog.versions == NULL && catalog.issue.offset == 2);
    assert(strcmp(catalog.issue.path,
        "generated/context-invalid/packages/test/fixture/v1.0.0/package.ini") == 0);
    destroy_catalog();
}
