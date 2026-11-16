/** @file
 * @brief Verify captured-registry ownership, collision diagnostics and every failed copy prefix.
 */
/** Caller source records and mutable names can expire after copying; catalog bytes and diagnostics remain owned. */
static void it_owns_captured_source_snapshots_without_rereading_files(void) {
    setup_catalog();
    const SourceText *text = NULL;
    assert(source_manager_add(&sources,"not-on-disk/value",(const unsigned char *)"@!\n",3,&text) == SOURCE_OK);
    char name[] = ".value";
    ModuleSnapshotEntry entry = {name,text,1,2};
    assert(module_catalog_snapshot(&catalog,&entry,1) == MODULE_OK);
    name[1] = 'X';
    source_manager_destroy(&sources);
    const ModuleSource *copy = NULL;
    assert(module_catalog_find(&catalog,".value",&copy) == MODULE_OK);
    assert(strcmp(source_path(copy->text),"not-on-disk/value") == 0);
    assert(source_length(copy->text) == 3 && memcmp(source_bytes(copy->text),"@!\n",3) == 0);
    destroy_catalog();
}

/** Both conflicting identities and invalid UTF-8 diagnostics survive destruction of their input source manager. */
static void it_retains_captured_registry_collision_and_encoding_diagnostics(void) {
    for (unsigned same_name = 0; same_name <= 1; same_name++) {
        setup_catalog();
        const SourceText *left = NULL, *right = NULL;
        assert(source_manager_add(&sources,"capture/left",(const unsigned char *)"@!\n",3,&left) == SOURCE_OK);
        assert(source_manager_add(&sources,"capture/right",(const unsigned char *)"@!\n",3,&right) == SOURCE_OK);
        ModuleSnapshotEntry entries[] = {{".left",left,1,1},{same_name ? ".left" : ".right",right,1,1}};
        assert(module_catalog_snapshot(&catalog,entries,2) == (same_name ? MODULE_AMBIGUOUS : MODULE_FILE_ALIAS));
        source_manager_destroy(&sources);
        assert(!catalog.built && strcmp(catalog.issue.path,"capture/right") == 0);
        assert(strcmp(catalog.issue.related_path,"capture/left") == 0);
        destroy_catalog();
    }
    setup_catalog();
    const unsigned char invalid[] = {255};
    const SourceText *text = NULL;
    assert(source_manager_add(&sources,"capture/invalid",invalid,1,&text) == SOURCE_OK);
    ModuleSnapshotEntry entry = {".invalid",text,1,1};
    assert(module_catalog_snapshot(&catalog,&entry,1) == MODULE_INVALID_UTF8);
    source_manager_destroy(&sources);
    assert(!catalog.built && catalog.issue.offset == 0 && source_bytes(catalog.issue.source)[0] == 255);
    destroy_catalog();
}

/** Every failed snapshot copy leaves no consumable registry and preserves caller-owned bytes. */
static void it_releases_every_failed_captured_registry_prefix(void) {
    setup_catalog();
    const SourceText *text = NULL;
    assert(source_manager_add(&sources,"capture/value",(const unsigned char *)"@!\n",3,&text) == SOURCE_OK);
    ModuleSnapshotEntry entry = {".value",text,1,1};
    size_t original_allocations = live_allocations;
    allocation_count = 0;
    assert(module_catalog_snapshot(&catalog,&entry,1) == MODULE_OK);
    size_t measured = allocation_count;
    module_catalog_destroy(&catalog);
    assert(live_allocations == original_allocations);
    for (size_t ordinal = 1; ordinal <= measured; ordinal++) {
        module_catalog_init(&catalog,sources.allocator);
        allocation_count = 0;
        failure_at = ordinal;
        assert(module_catalog_snapshot(&catalog,&entry,1) == MODULE_OUT_OF_MEMORY && !catalog.built);
        module_catalog_destroy(&catalog);
        assert(live_allocations == original_allocations && source_bytes(text)[0] == '@');
    }
    failure_at = 0;
    module_catalog_init(&catalog,sources.allocator);
    destroy_catalog();
}
