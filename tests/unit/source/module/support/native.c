/** @file
 * @brief Verify explicit native companion publication, memoization and failure ownership.
 */
/** Prepare exact optional companion paths without requiring module-body parsing in the source layer. */
static void prepare_native_publication(void) {
    directory("generated/native publication");
    file("generated/native publication/item", "@C\n");
    file("generated/native publication/item.c", "/* native body */\n");
    file("generated/native publication/item.h", "/* native header */\n");
    file("generated/native publication/item.abi", "+ initialize\n- destroy\n");
}
/** Companions publish atomically only on explicit request; successful requests reuse identical catalog-owned views without allocation. */
static void it_publishes_native_companions_once_after_explicit_selection(void) {
    prepare_native_publication();
    setup_catalog();
    ModuleRoot root = {"generated/native publication", NULL, 0, MODULE_ROOT_PROJECT};
    ModuleStatus status = module_catalog_load_native(&catalog, NULL);
    assert(status == MODULE_INVALID_PATH);
    status = module_catalog_build(&catalog, &root, 1);
    assert(status == MODULE_OK);
    const ModuleSource *source = NULL;
    status = module_catalog_find(&catalog, ".item", &source);
    assert(status == MODULE_OK && !source->native_loaded && source->native_c == NULL && source->native_abi == NULL);
    ModuleSource foreign = {0};
    status = module_catalog_load_native(&catalog, &foreign);
    assert(status == MODULE_INVALID_PATH && !source->native_loaded);
    status = module_catalog_load_native(&catalog, source);
    assert(status == MODULE_OK && source->native_loaded);
    const char *body = source->native_c, *header = source->native_header;
    const SourceText *abi = source->native_abi;
    assert(body != NULL && header != NULL && abi != NULL);
    size_t allocations = allocation_count;
    status = module_catalog_load_native(&catalog, source);
    assert(status == MODULE_OK && allocation_count == allocations);
    assert(source->native_c == body && source->native_header == header && source->native_abi == abi);
    destroy_catalog();
}
/** Every failed native selection retains all scratch under the catalog and leaves every public companion field absent. */
static void it_unwinds_every_native_companion_allocation_without_partial_publication(void) {
    prepare_native_publication();
    ModuleRoot root = {"generated/native publication", NULL, 0, MODULE_ROOT_PROJECT};
    setup_catalog();
    ModuleStatus status = module_catalog_build(&catalog, &root, 1);
    assert(status == MODULE_OK);
    const ModuleSource *source = NULL;
    status = module_catalog_find(&catalog, ".item", &source);
    assert(status == MODULE_OK);
    size_t begin = allocation_count;
    status = module_catalog_load_native(&catalog, source);
    assert(status == MODULE_OK);
    size_t total = allocation_count - begin;
    destroy_catalog();
    for (size_t failure = 1; failure <= total; failure++) {
        setup_catalog();
        status = module_catalog_build(&catalog, &root, 1);
        assert(status == MODULE_OK);
        status = module_catalog_find(&catalog, ".item", &source);
        assert(status == MODULE_OK);
        failure_at = allocation_count + failure;
        status = module_catalog_load_native(&catalog, source);
        assert(status == MODULE_OUT_OF_MEMORY && !source->native_loaded);
        assert(source->native_c == NULL && source->native_header == NULL && source->native_abi == NULL);
        destroy_catalog();
    }
}
