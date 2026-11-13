/**
 * @file
 * @brief Compose multiword plus-directory storage, exact native companions, root-family aliases and failure ownership.
 */

/** Prepare independent nested storage; a draft parent anchors physical layout without becoming an implicit dependency. */
static void prepare_spaced_modules(void) {
    directory("generated/根 目录");
    directory("generated/根 目录/商店 工具");
    file("generated/根 目录/商店 工具/商品 状态", "unfinished parent\n");
    directory("generated/根 目录/商店 工具/商品 状态+");
    file("generated/根 目录/商店 工具/商品 状态+/读取 数据", "@C\r\n");
    file("generated/根 目录/商店 工具/商品 状态+/读取 数据.c", "/* body */\n");
    file("generated/根 目录/商店 工具/商品 状态+/读取 数据.h", "/* header */\n");
    file("generated/根 目录/商店 工具/商品 状态+/读取 数据.abi", "+ initialize\n- destroy\n");
    directory("generated/根 目录/商店 工具/商品 状态+/读取 数据+");
    file("generated/根 目录/商店 工具/商品 状态+/读取 数据+/详情 项目", "@!\n");
    file("generated/根 目录/商店 工具/README.md", "not a module\n");
}

/** Strip structural plus only from directories, preserve native physical locations and never inherit/execute a parent. */
static void it_maps_nested_multiword_submodules_and_native_companions_in_each_root_kind(void) {
    prepare_spaced_modules();
    const ModuleRootKind kinds[] = {MODULE_ROOT_PROJECT,MODULE_ROOT_STANDARD};
    const char *paths[] = {".商店 工具.商品 状态.读取 数据", "..商店 工具.商品 状态.读取 数据"};
    const char *nested[] = {".商店 工具.商品 状态.读取 数据.详情 项目", "..商店 工具.商品 状态.读取 数据.详情 项目"};
    for (size_t selected = 0; selected < 2; selected++) {
        setup_catalog();
        ModuleRoot root = {.path = "generated/根 目录", .kind = kinds[selected]};
        assert(module_catalog_build(&catalog, &root, 1) == MODULE_OK);
        const ModuleSource *source = NULL;
        assert(module_catalog_find(&catalog, paths[selected], &source) == MODULE_OK);
        assert(strcmp(source_path(source->text), "generated/根 目录/商店 工具/商品 状态+/读取 数据") == 0);
        ModuleStatus selected_native = module_catalog_load_native(&catalog, source);
        assert(selected_native == MODULE_OK);
        assert(strcmp(source->native_c, "generated/根 目录/商店 工具/商品 状态+/读取 数据.c") == 0);
        assert(strcmp(source->native_header, "generated/根 目录/商店 工具/商品 状态+/读取 数据.h") == 0);
        assert(strcmp(source_path(source->native_abi), "generated/根 目录/商店 工具/商品 状态+/读取 数据.abi") == 0);
        assert(module_catalog_find(&catalog, nested[selected], &source) == MODULE_OK);
        assert(strstr(source_path(source->text), "商品 状态+/读取 数据+/详情 项目") != NULL);
        destroy_catalog();
    }
}

/** Any same-family duplicate or physical alias fails both root orders with both physical paths retained. */
static void it_rejects_conflicting_standard_roots_and_cross_family_physical_aliases(void) {
    directory("generated/duplicate first");
    directory("generated/duplicate second");
    file("generated/duplicate first/item state", "@!\n");
    file("generated/duplicate second/item state", "@!\n");
    ModuleRoot roots[] = {{"generated/duplicate first",NULL,0,MODULE_ROOT_STANDARD},
        {"generated/duplicate second",NULL,0,MODULE_ROOT_STANDARD}};
    for (size_t order = 0; order < 2; order++) {
        setup_catalog();
        assert(module_catalog_build(&catalog, roots, 2) == MODULE_AMBIGUOUS && !catalog.built);
        assert(catalog.issue.path != NULL && catalog.issue.related_path != NULL);
        assert(strcmp(catalog.issue.path, catalog.issue.related_path) != 0);
        assert(strstr(catalog.issue.path, "item state") != NULL && strstr(catalog.issue.related_path, "item state") != NULL);
        destroy_catalog();
        ModuleRoot temporary = roots[0];
        roots[0] = roots[1];
        roots[1] = temporary;
    }
    int removed = unlink("generated/duplicate second/item state");
    assert(removed == 0);
    assert(link("generated/duplicate first/item state", "generated/duplicate second/item state") == 0);
    roots[0].kind = MODULE_ROOT_PROJECT;
    for (size_t order = 0; order < 2; order++) {
        setup_catalog();
        assert(module_catalog_build(&catalog, roots, 2) == MODULE_FILE_ALIAS && !catalog.built);
        assert(catalog.issue.path != NULL && catalog.issue.related_path != NULL);
        assert(strcmp(catalog.issue.path, catalog.issue.related_path) != 0);
        destroy_catalog();
        ModuleRoot temporary = roots[0];
        roots[0] = roots[1];
        roots[1] = temporary;
    }
}

/** Missing or nonregular exact anchors remain an error even when their multiword bases are otherwise valid. */
static void it_reports_exact_missing_multiword_parent_anchors_without_alternate_resolution(void) {
    directory("generated/spaced orphan");
    directory("generated/spaced orphan/parent module+");
    file("generated/spaced orphan/parent module+/child module", "@!\n");
    setup_catalog();
    ModuleRoot root = {.path = "generated/spaced orphan", .kind = MODULE_ROOT_STANDARD};
    assert(module_catalog_build(&catalog, &root, 1) == MODULE_ORPHAN_DIRECTORY);
    assert(strcmp(catalog.issue.path, "generated/spaced orphan/parent module") == 0);
    destroy_catalog();
}

/** Every measured extended-root discovery failure leaves no usable partial catalog or owned scratch. */
static void it_unwinds_each_multiword_standard_catalog_allocation_failure(void) {
    prepare_spaced_modules();
    ModuleRoot root = {.path = "generated/根 目录", .kind = MODULE_ROOT_STANDARD};
    setup_catalog();
    assert(module_catalog_build(&catalog, &root, 1) == MODULE_OK);
    size_t count = allocation_count;
    destroy_catalog();
    for (size_t failure = 1; failure <= count; failure++) {
        setup_catalog();
        failure_at = failure;
        assert(module_catalog_build(&catalog, &root, 1) == MODULE_OUT_OF_MEMORY && !catalog.built);
        const ModuleSource *unpublished = NULL;
        assert(module_catalog_find(&catalog, "..商店 工具.商品 状态.读取 数据", &unpublished) == MODULE_INVALID_PATH);
        assert(unpublished == NULL);
        destroy_catalog();
    }
}
