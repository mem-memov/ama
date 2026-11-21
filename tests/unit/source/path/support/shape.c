/**
 * @file
 * @brief Verify canonical prefix categories and exact version/source-area metadata.
 * [Path/scope contract](../../../../../docs/implementation/project-configuration.md#t261-path-and-scope-contract)
 * requires structural facts without probe modules, normalization or file lookup.
 */
/** Valid namespace/version/area prefixes remain distinct from complete module identities. */
static void it_describes_exact_root_version_area_and_module_boundaries(void) {
    const struct {
        const char *text;
        ModuleRootKind root;
        ModulePathExtent extent;
        ModulePathArea area;
        size_t components;
        size_t version_components;
        const char *version;
    } cases[] = {
        {"/", MODULE_ROOT_PROJECT, MODULE_PATH_NAMESPACE, MODULE_PATH_NO_AREA, 0, 0, ""},
        {"//", MODULE_ROOT_STANDARD, MODULE_PATH_NAMESPACE, MODULE_PATH_NO_AREA, 0, 0, ""},
        {"/acme", MODULE_ROOT_PROJECT, MODULE_PATH_NAMESPACE, MODULE_PATH_NO_AREA, 1, 0, ""},
        {"/acme/pkg", MODULE_ROOT_PROJECT, MODULE_PATH_NAMESPACE, MODULE_PATH_NO_AREA, 2, 0, ""},
        {"/acme/pkg/v1.2.3", MODULE_ROOT_PROJECT, MODULE_PATH_VERSION, MODULE_PATH_NO_AREA, 3, 3, "/acme/pkg/v1.2.3"},
        {"/acme/pkg/v1.2.3/src", MODULE_ROOT_PROJECT, MODULE_PATH_AREA, MODULE_PATH_SRC, 4, 3, "/acme/pkg/v1.2.3"},
        {"/acme/pkg/v1.2.3/tests/case", MODULE_ROOT_PROJECT, MODULE_PATH_MODULE, MODULE_PATH_TESTS, 5, 3, "/acme/pkg/v1.2.3"},
        {"/acme/src/pkg/v1.2.3/tests/module", MODULE_ROOT_PROJECT, MODULE_PATH_MODULE, MODULE_PATH_TESTS, 6, 4, "/acme/src/pkg/v1.2.3"},
        {"//tests", MODULE_ROOT_STANDARD, MODULE_PATH_MODULE, MODULE_PATH_NO_AREA, 1, 0, ""},
        {"//a/src/tests", MODULE_ROOT_STANDARD, MODULE_PATH_MODULE, MODULE_PATH_NO_AREA, 3, 0, ""},
        {"/名字/包/v0.0.0/src/名", MODULE_ROOT_PROJECT, MODULE_PATH_MODULE, MODULE_PATH_SRC, 5, 3, "/名字/包/v0.0.0"}
    };
    _Static_assert(MODULE_ROOT_PROJECT == 1 && MODULE_ROOT_STANDARD == 2, "Existing root-family values stay stable");
    for (size_t index = 0; index < sizeof(cases) / sizeof(*cases); index++) {
        ModulePathShape shape;
        assert(module_path_describe((const unsigned char *)cases[index].text, strlen(cases[index].text), &shape));
        assert(shape.root == cases[index].root && shape.extent == cases[index].extent);
        assert(shape.area == cases[index].area && shape.components == cases[index].components);
        assert(shape.version_components == cases[index].version_components);
        assert(shape.version_length == strlen(cases[index].version));
        assert(module_path_canonical((const unsigned char *)cases[index].text, strlen(cases[index].text))
            == (cases[index].extent == MODULE_PATH_MODULE));
    }
}

/** Wrong roots, dot traversal, misplaced versions and padding cannot publish a partial shape. */
static void it_preserves_shape_output_on_invalid_or_unbounded_endpoint_requests(void) {
    const char *invalid[] = {"", "a", "///", "///a", "/a/", "//a/", "/a//b", "//a//b", "/a/.", "/a/..",
        "//a/..", "~a", ".type", "./a", "../a", "/a/v1.2.3", "//v1.2.3", "/a/pkg/v1.2.3/other",
        "/a/pkg/v1.2.3/src/v2.3.4", " /a", "/a ", "/a/pkg/v1.2.3/src/a--b", "/a/\xff"};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        ModulePathShape shape = {.root = MODULE_ROOT_STANDARD, .extent = MODULE_PATH_MODULE,
            .area = MODULE_PATH_TESTS, .components = 17, .version_components = 13, .version_length = 29};
        unsigned char saved[sizeof(shape)];
        memcpy(saved, &shape, sizeof(shape));
        assert(!module_path_describe((const unsigned char *)invalid[index], strlen(invalid[index]), &shape));
        assert(memcmp(saved, &shape, sizeof(shape)) == 0);
    }
    ModulePathShape shape = {0};
    assert(!module_path_describe(NULL, 0, &shape));
    assert(!module_path_describe(NULL, 100, &shape));
    assert(!module_path_describe((const unsigned char *)"//a", 3, NULL));
    static const unsigned char bytes[] = "/a/pkg/v0.0.0/src/x\0tail";
    size_t endpoint = strlen((const char *)bytes);
    assert(module_path_describe(bytes, endpoint, &shape) && shape.extent == MODULE_PATH_MODULE);
    assert(!module_path_describe(bytes, sizeof(bytes) - 1, &shape));
    const unsigned char bounded[] = {'/', '/', 'x'};
    assert(module_path_describe(bounded, sizeof(bounded), &shape) && shape.components == 1);
}

/** Independent construction covers all package depths 1..3, both areas and module depths 0..3. */
static void it_classifies_every_bounded_version_and_area_shape_without_probe_names(void) {
    const char *areas[] = {"src", "tests"};
    for (size_t packages = 1; packages <= 3; packages++) {
        char version[128] = "/acme";
        for (size_t index = 0; index < packages; index++) { strcat(version, "/pkg"); }
        strcat(version, "/v2.4.14-speed");
        ModulePathShape shape;
        assert(module_path_describe((const unsigned char *)version, strlen(version), &shape));
        assert(shape.extent == MODULE_PATH_VERSION && shape.components == packages + 2);
        for (size_t area = 0; area < 2; area++) {
            for (size_t modules = 0; modules <= 3; modules++) {
                char text[256];
                int written = snprintf(text, sizeof(text), "%s/%s", version, areas[area]);
                assert(written > 0 && (size_t)written < sizeof(text));
                for (size_t index = 0; index < modules; index++) { strcat(text, "/item"); }
                assert(module_path_describe((const unsigned char *)text, strlen(text), &shape));
                assert(shape.extent == (modules == 0 ? MODULE_PATH_AREA : MODULE_PATH_MODULE));
                assert(shape.root == MODULE_ROOT_PROJECT && shape.components == packages + 3 + modules);
                assert(shape.version_length == strlen(version) && shape.version_components == packages + 2);
                assert(shape.area == (area == 0 ? MODULE_PATH_SRC : MODULE_PATH_TESTS));
            }
        }
    }
}
