/** @file
 * @brief Check explicit candidate-source publication, retained old bytes and each failed copying prefix.
 */
/** A candidate changes only selected text; invalid UTF-8 retains its diagnostic source without changing the selected record. */
static void it_overrides_candidate_text_without_changing_identity_or_old_source_views(void) {
    prepare_package("generated/overlay");
    file("generated/overlay/packages/test/fixture/v1.0.0/src/item","@!\n");
    file("generated/overlay/packages/test/fixture/v1.0.0/src/native","@C\n");
    setup_catalog();
    ModuleRoot root = {"generated/overlay",NULL,0,MODULE_ROOT_PROJECT};
    assert(module_catalog_build(&catalog,&root,1) == MODULE_OK);
    const ModuleSource *item = NULL, *native = NULL;
    assert(module_catalog_find(&catalog,"/test/fixture/v1.0.0/src/item",&item) == MODULE_OK);
    const SourceText *before = item->text;
    dev_t device = item->device;
    ino_t inode = item->inode;
    const unsigned char replacement[] = "@!\nvalue\n.type\n";
    assert(module_catalog_override(&catalog,item,replacement,sizeof(replacement)-1) == MODULE_OK);
    assert(item->text != before && strcmp((const char *)source_bytes(before),"@!\n") == 0);
    assert(strcmp((const char *)source_bytes(item->text),(const char *)replacement) == 0);
    assert(strcmp(source_path(item->text),source_path(before)) == 0 && item->device == device && item->inode == inode);
    const SourceText *accepted = item->text;
    const unsigned char invalid[] = {255};
    assert(module_catalog_override(&catalog,item,invalid,sizeof(invalid)) == MODULE_INVALID_UTF8 && item->text == accepted);
    assert(catalog.issue.source != NULL && catalog.issue.offset == 0 && source_bytes(catalog.issue.source)[0] == 255);
    assert(module_catalog_find(&catalog,"/test/fixture/v1.0.0/src/native",&native) == MODULE_OK);
    assert(module_catalog_load_native(&catalog,native) == MODULE_OK);
    assert(module_catalog_override(&catalog,native,replacement,sizeof(replacement)-1) == MODULE_INVALID_PATH);
    destroy_catalog();
}
/** Failed candidate publication cannot retarget its record or leak an incomplete source/path/line allocation. */
static void it_unwinds_every_candidate_source_override_allocation(void) {
    const unsigned char bytes[] = "@!\nnew value\n.type\n";
    ModuleRoot root = {"generated/overlay",NULL,0,MODULE_ROOT_PROJECT};
    setup_catalog();
    assert(module_catalog_build(&catalog,&root,1) == MODULE_OK);
    const ModuleSource *item = NULL;
    assert(module_catalog_find(&catalog,"/test/fixture/v1.0.0/src/item",&item) == MODULE_OK);
    size_t baseline = allocation_count;
    assert(module_catalog_override(&catalog,item,bytes,sizeof(bytes)-1) == MODULE_OK);
    size_t total = allocation_count-baseline;
    destroy_catalog();
    for (size_t failure = 1; failure <= total; failure++) {
        setup_catalog();
        assert(module_catalog_build(&catalog,&root,1) == MODULE_OK);
        assert(module_catalog_find(&catalog,"/test/fixture/v1.0.0/src/item",&item) == MODULE_OK);
        const SourceText *before = item->text;
        failure_at = allocation_count+failure;
        assert(module_catalog_override(&catalog,item,bytes,sizeof(bytes)-1) == MODULE_OUT_OF_MEMORY);
        assert(item->text == before && catalog.issue.status == MODULE_OUT_OF_MEMORY);
        destroy_catalog();
    }
}
