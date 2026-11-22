/** @file
 * @brief Verify complete projected catalogs, companion ownership and refusal without disk fallback.
 * @see [T224 contract](../../../../../docs/implementation/source-operations.md)
 */
#include "common.c"
/** Normal common selection/indexing accepts a purely virtual view and its lazy
 * companions, keeps local precedence and owns bytes at the new physical keys. */
static void it_indexes_complete_virtual_catalogs_and_lazy_companions_without_physical_reads(void) {
    setup_test(); ViewModel model; model_init(&model); ModuleCatalogReader reader = reader_for(&model);
    ModuleCatalog catalog; module_catalog_init(&catalog, sources.allocator);
    assert(prepare_catalog(&catalog, &reader) == MODULE_OK && catalog.built && catalog.reader == &reader);
    const ModuleSource *state = NULL, *child_source = NULL, *standard = NULL;
    assert(module_catalog_find(&catalog, "/vendor/pkg/v1.0.0/src/state", &state) == MODULE_OK);
    assert(module_catalog_find(&catalog, "/vendor/pkg/v1.0.0/src/state/child", &child_source) == MODULE_OK);
    assert(module_catalog_find(&catalog, "//integer", &standard) == MODULE_OK);
    assert(state->text != model.texts[8] && !strcmp(source_path(state->text), nodes[8].path));
    assert(state->configuration && state->configuration != model.texts[11]);
    assert(!strcmp(source_path(state->configuration), nodes[11].path));
    assert(state->native_loaded && state->native_c && state->native_header && state->native_abi);
    assert(!strcmp(state->native_c, nodes[12].path) && !strcmp(state->native_header, nodes[13].path));
    assert(source_length(state->native_abi) == 0 && state->native_abi != model.texts[14]);
    assert(child_source->configuration_loaded && !child_source->configuration && catalog.issue.status == MODULE_OK);
    assert(model.reads == 7); /* Main/config/ABI bytes only; opaque C/h are not text reads. */
    assert(standard->version == NULL && state->version && state->version->configuration);
    size_t calls = model.calls;
    assert(module_catalog_load_configuration(&catalog, state) == MODULE_OK && module_catalog_load_native(&catalog, state) == MODULE_OK);
    assert(model.calls == calls);
    assert(module_catalog_build_view(&catalog, NULL, 0, &reader) == MODULE_INVALID_PATH && catalog.built);
    module_catalog_destroy(&catalog); assert(!catalog.reader); destroy_test();
}
/** Every metadata/list/text callback can fail; neither original selection nor
 * native/configuration observation may retry in the host filesystem. */
static void it_unwinds_every_virtual_catalog_reader_callback_failure_without_disk_fallback(void) {
    size_t calls = 0;
    for (size_t failure = 0; !failure || failure <= calls; failure++) {
        setup_test(); ViewModel model; model_init(&model); model.fail_at = failure;
        ModuleCatalogReader reader = reader_for(&model); ModuleCatalog catalog; module_catalog_init(&catalog, sources.allocator);
        ModuleStatus status = prepare_catalog(&catalog, &reader);
        if (!failure) { assert(status == MODULE_OK); calls = model.calls; assert(calls && model.reads); }
        else { assert(status == MODULE_IO_ERROR && catalog.issue.status == MODULE_IO_ERROR); }
        module_catalog_destroy(&catalog); destroy_test();
    }
}
/** Exhaust actual owner allocation failures with a separately prepared immutable
 * reader domain, including directory-name arrays and deep source/companion copies. */
static void it_retires_every_complete_virtual_catalog_allocation_failure(void) {
    setup_test(); ViewModel model; model_init(&model); ModuleCatalogReader reader = reader_for(&model);
    size_t allocations = 0, baseline = live_allocations;
    for (size_t failure = 0; !failure || failure <= allocations; failure++) {
        model.calls = model.reads = 0;
        ModuleCatalog catalog; module_catalog_init(&catalog, sources.allocator);
        size_t before = allocation_count; if (failure) { failure_at = before + failure; }
        ModuleStatus status = prepare_catalog(&catalog, &reader);
        if (!failure) { assert(status == MODULE_OK); allocations = allocation_count - before; assert(allocations); }
        else { assert(status == MODULE_OUT_OF_MEMORY && catalog.issue.status == MODULE_OUT_OF_MEMORY); }
        failure_at = 0; module_catalog_destroy(&catalog); assert(live_allocations == baseline);
    }
    destroy_test();
}
/** The common alias, orphan-anchor, required-config and UTF-8 checks remain
 * authoritative; a valid callback status with no bytes is not accepted. */
static void it_applies_common_alias_anchor_configuration_and_encoding_rules_to_virtual_views(void) {
    const ModuleStatus expected[] = {MODULE_FILE_ALIAS, MODULE_FILE_ALIAS, MODULE_ORPHAN_DIRECTORY,
        MODULE_MISSING, MODULE_INVALID_UTF8, MODULE_INVALID_PATH};
    for (size_t profile = 0; profile < sizeof(expected) / sizeof(expected[0]); profile++) {
        setup_test(); ViewModel model; model_init(&model);
        model.source_alias = profile == 0; model.directory_alias = profile == 1; model.hide_state = profile == 2;
        model.hide_configuration = profile == 3; model.bad_utf8 = profile == 4; model.null_read = profile == 5;
        ModuleCatalogReader reader = reader_for(&model); ModuleCatalog catalog; module_catalog_init(&catalog, sources.allocator);
        assert(prepare_catalog(&catalog, &reader) == expected[profile] && !catalog.built);
        if (profile == 4) { assert(catalog.issue.source && catalog.issue.offset == 0 && source_bytes(catalog.issue.source)[0] == 0xff); }
        module_catalog_destroy(&catalog); destroy_test();
    }
}
/** Incomplete read contracts cannot partially install a view or consume the
 * owner's one build attempt; empty complete views remain legitimate. */
static void it_rejects_incomplete_catalog_readers_before_attempt_and_accepts_empty_views(void) {
    setup_test(); ViewModel model; model_init(&model); ModuleCatalogReader reader = reader_for(&model);
    for (unsigned missing = 0; missing < 5; missing++) {
        ModuleCatalogReader bad = reader;
        if (missing == 1) { bad.tree.root = NULL; }
        if (missing == 2) { bad.tree.child = NULL; }
        if (missing == 3) { bad.tree.entries = NULL; }
        if (missing == 4) { bad.read = NULL; }
        ModuleCatalog catalog; module_catalog_init(&catalog, sources.allocator);
        assert(module_catalog_build_view(&catalog, NULL, 0, missing ? &bad : NULL) == MODULE_INVALID_PATH);
        assert(!catalog.attempted && !catalog.reader);
        assert(module_catalog_build_view(&catalog, NULL, 0, &reader) == MODULE_OK && catalog.built && !catalog.first);
        module_catalog_destroy(&catalog);
    }
    assert(module_catalog_build_view(NULL, NULL, 0, &reader) == MODULE_INVALID_PATH);
    destroy_test();
}
/** Execute view, companion, finite fault-domain and invariant boundaries. */
void run(void) {
    it_indexes_complete_virtual_catalogs_and_lazy_companions_without_physical_reads();
    it_unwinds_every_virtual_catalog_reader_callback_failure_without_disk_fallback();
    it_retires_every_complete_virtual_catalog_allocation_failure();
    it_applies_common_alias_anchor_configuration_and_encoding_rules_to_virtual_views();
    it_rejects_incomplete_catalog_readers_before_attempt_and_accepts_empty_views();
}
/** All borrowed view inputs outlive catalog teardown. */
int main(void) { run(); return 0; }
