/** @file
 * @brief Check active module configuration observations, captured authority and every failed copy.
 * The enclosing catalog leaf owns physical fixtures and counted allocation teardown.
 */

/** Native and alias observations share exact probing while local files remain per-module. */
static void it_captures_local_configuration_presence_once_without_submodule_inheritance(void) {
    prepare_package("generated/local-configuration");
    const char *base = "generated/local-configuration/packages/test/fixture/v1.0.0/src/item";
    file(base, "@C\n");
    file("generated/local-configuration/packages/test/fixture/v1.0.0/src/item.ini", "; retained\r\n");
    directory("generated/local-configuration/packages/test/fixture/v1.0.0/src/item+");
    file("generated/local-configuration/packages/test/fixture/v1.0.0/src/item+/child", "@!\n");
    ModuleRoot root = {.path = "generated/local-configuration", .kind = MODULE_ROOT_PROJECT};
    setup_catalog();
    assert(module_catalog_build(&catalog, &root, 1) == MODULE_OK);
    const ModuleSource *parent = NULL, *child = NULL;
    assert(module_catalog_find(&catalog, "/test/fixture/v1.0.0/src/item", &parent) == MODULE_OK);
    assert(module_catalog_find(&catalog, "/test/fixture/v1.0.0/src/item/child", &child) == MODULE_OK);
    assert(!parent->configuration_loaded && !child->configuration_loaded);
    assert(module_catalog_load_configuration(&catalog, parent) == MODULE_OK);
    assert(parent->configuration_loaded && parent->configuration != NULL);
    assert(strcmp(parent->configuration_path,
        "generated/local-configuration/packages/test/fixture/v1.0.0/src/item.ini") == 0);
    const SourceText *saved = parent->configuration;
    file(parent->configuration_path, "changed");
    failure_at = allocation_count + 1;
    assert(module_catalog_load_configuration(&catalog, parent) == MODULE_OK);
    assert(parent->configuration == saved && source_length(saved) == 12);
    failure_at = 0;
    assert(module_catalog_load_configuration(&catalog, child) == MODULE_OK);
    assert(child->configuration_loaded && child->configuration == NULL);
    file(child->configuration_path, "");
    assert(module_catalog_load_configuration(&catalog, child) == MODULE_OK && child->configuration == NULL);
    assert(!parent->native_loaded);
    assert(module_catalog_load_native(&catalog, parent) == MODULE_OK && parent->native_loaded);
    assert(parent->native_c == NULL && parent->configuration == saved);
    ModuleSource foreign = {0};
    assert(module_catalog_load_configuration(NULL, parent) == MODULE_INVALID_PATH);
    assert(module_catalog_load_configuration(&catalog, NULL) == MODULE_INVALID_PATH);
    assert(module_catalog_load_configuration(&catalog, &foreign) == MODULE_INVALID_PATH);
    destroy_catalog();
}

/** Snapshot absence is authoritative even if the anticipated physical companion exists. */
static void it_owns_optional_snapshot_configuration_and_never_falls_back_to_disk(void) {
    prepare_package("generated/snapshot-alias");
    file("generated/snapshot-alias/packages/test/fixture/v1.0.0/src/value.ini", "\xff");
    setup_catalog();
    const SourceText *text = NULL, *configuration = NULL;
    assert(source_manager_add(&sources,
        "generated/snapshot-alias/packages/test/fixture/v1.0.0/src/value",
        (const unsigned char *)"@!\n", 3, &text) == SOURCE_OK);
    const unsigned char bytes[] = {';', 0, '\r', '\n'};
    assert(source_manager_add(&sources, "absent/copied.ini", bytes, sizeof(bytes), &configuration) == SOURCE_OK);
    ModuleSnapshotEntry entries[] = {
        {"//absent", text, 1, 1, NULL},
        {"//present", text, 1, 2, configuration}
    };
    assert(module_catalog_snapshot(&catalog, entries, 2) == MODULE_OK);
    source_manager_destroy(&sources);
    const ModuleSource *absent = NULL, *present = NULL;
    assert(module_catalog_find(&catalog, "//absent", &absent) == MODULE_OK);
    assert(module_catalog_find(&catalog, "//present", &present) == MODULE_OK);
    assert(module_catalog_load_configuration(&catalog, absent) == MODULE_OK);
    assert(absent->configuration_loaded && absent->configuration == NULL);
    assert(module_catalog_load_configuration(&catalog, present) == MODULE_OK);
    assert(present->configuration_loaded && present->version == NULL);
    assert(strcmp(present->configuration_path, "absent/copied.ini") == 0);
    assert(source_length(present->configuration) == sizeof(bytes));
    assert(memcmp(source_bytes(present->configuration), bytes, sizeof(bytes)) == 0);
    destroy_catalog();
}

/** Every physical configuration load allocation can fail without partial cached publication. */
static void it_unwinds_every_local_configuration_observation_allocation(void) {
    prepare_package("generated/alias-failure");
    file("generated/alias-failure/packages/test/fixture/v1.0.0/src/value", "@!\n");
    file("generated/alias-failure/packages/test/fixture/v1.0.0/src/value.ini", "; fixture\n");
    ModuleRoot root = {.path = "generated/alias-failure", .kind = MODULE_ROOT_PROJECT};
    setup_catalog();
    assert(module_catalog_build(&catalog, &root, 1) == MODULE_OK);
    const ModuleSource *source = NULL;
    assert(module_catalog_find(&catalog, "/test/fixture/v1.0.0/src/value", &source) == MODULE_OK);
    allocation_count = 0;
    assert(module_catalog_load_configuration(&catalog, source) == MODULE_OK);
    size_t measured = allocation_count;
    destroy_catalog();
    for (size_t ordinal = 1; ordinal <= measured; ordinal++) {
        setup_catalog();
        assert(module_catalog_build(&catalog, &root, 1) == MODULE_OK);
        assert(module_catalog_find(&catalog, "/test/fixture/v1.0.0/src/value", &source) == MODULE_OK);
        allocation_count = 0;
        failure_at = ordinal;
        assert(module_catalog_load_configuration(&catalog, source) == MODULE_OUT_OF_MEMORY);
        assert(!source->configuration_loaded && source->configuration == NULL && source->configuration_path == NULL);
        destroy_catalog();
    }
}

/** Unsupported companions and invalid UTF-8 preserve exact owned failures without publication. */
static void it_rejects_invalid_local_configuration_files_and_captured_utf8(void) {
    prepare_package("generated/alias-invalid");
    file("generated/alias-invalid/packages/test/fixture/v1.0.0/src/value", "@!\n");
    const char *path = "generated/alias-invalid/packages/test/fixture/v1.0.0/src/value.ini";
    directory(path);
    ModuleRoot root = {.path = "generated/alias-invalid", .kind = MODULE_ROOT_PROJECT};
    setup_catalog();
    assert(module_catalog_build(&catalog, &root, 1) == MODULE_OK);
    const ModuleSource *source = NULL;
    assert(module_catalog_find(&catalog, "/test/fixture/v1.0.0/src/value", &source) == MODULE_OK);
    assert(module_catalog_load_configuration(&catalog, source) == MODULE_UNSUPPORTED_PATH);
    assert(strcmp(catalog.issue.path, path) == 0 && !source->configuration_loaded);
    assert(rmdir(path) == 0);
    file(path, ";\n\xff");
    assert(module_catalog_load_configuration(&catalog, source) == MODULE_INVALID_UTF8);
    assert(catalog.issue.offset == 2 && strcmp(catalog.issue.path, path) == 0);
    assert(source_bytes(catalog.issue.source)[2] == 255 && !source->configuration_loaded);
    assert(unlink(path) == 0);
    assert(symlink("value", path) == 0);
    assert(module_catalog_load_configuration(&catalog, source) == MODULE_UNSUPPORTED_PATH);
    assert(!source->configuration_loaded && source->configuration_path == NULL);
    destroy_catalog();

    setup_catalog();
    const SourceText *text = NULL, *bad = NULL;
    assert(source_manager_add(&sources, "absent/value", (const unsigned char *)"@!\n", 3, &text) == SOURCE_OK);
    assert(source_manager_add(&sources, "absent/value.ini", (const unsigned char *)";\n\xff", 3, &bad) == SOURCE_OK);
    ModuleSnapshotEntry entry = {"//value", text, 1, 1, bad};
    assert(module_catalog_snapshot(&catalog, &entry, 1) == MODULE_INVALID_UTF8);
    source_manager_destroy(&sources);
    assert(!catalog.built && catalog.first == NULL && catalog.issue.offset == 2);
    assert(strcmp(catalog.issue.path, "absent/value.ini") == 0 && source_length(catalog.issue.source) == 3);
    destroy_catalog();
}

/** Every present snapshot-copy allocation fails cleanly before publishing a source record. */
static void it_unwinds_every_present_configuration_snapshot_copy(void) {
    setup_catalog();
    const SourceText *text = NULL, *configuration = NULL;
    assert(source_manager_add(&sources, "absent/value", (const unsigned char *)"@!\n", 3, &text) == SOURCE_OK);
    assert(source_manager_add(&sources, "absent/value.ini", NULL, 0, &configuration) == SOURCE_OK);
    ModuleSnapshotEntry entry = {"//value", text, 1, 1, configuration};
    size_t baseline = live_allocations;
    allocation_count = 0;
    assert(module_catalog_snapshot(&catalog, &entry, 1) == MODULE_OK);
    size_t measured = allocation_count;
    module_catalog_destroy(&catalog);
    assert(live_allocations == baseline);
    for (size_t ordinal = 1; ordinal <= measured; ordinal++) {
        module_catalog_init(&catalog, sources.allocator);
        allocation_count = 0;
        failure_at = ordinal;
        assert(module_catalog_snapshot(&catalog, &entry, 1) == MODULE_OUT_OF_MEMORY);
        assert(!catalog.built && catalog.first == NULL && source_length(configuration) == 0);
        module_catalog_destroy(&catalog);
        assert(live_allocations == baseline);
    }
    failure_at = 0;
    module_catalog_init(&catalog, sources.allocator);
    destroy_catalog();
}
