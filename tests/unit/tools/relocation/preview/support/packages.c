/**
 * @file
 * @brief Compose projected moves with whole-version precedence and required local areas.
 */

/** Prepare a complete ignored external copy with a deliberately invalid source.
 * No selected local module or missing area may be filled from this copy.
 */
static void external_version(void) {
    folder("project/dependencies"); folder("project/dependencies/test");
    folder("project/dependencies/test/fixture");
    folder("project/dependencies/test/fixture/v1.0.0");
    folder("project/dependencies/test/fixture/v1.0.0/src");
    folder("project/dependencies/test/fixture/v1.0.0/tests");
    folder("project/dependencies/test/fixture/v1.0.0/src/old");
    folder("project/dependencies/test/fixture/v1.0.0/src/new");
    file("project/dependencies/test/fixture/v1.0.0/package.ini", "");
    file("project/dependencies/test/fixture/v1.0.0/src/old/extra", "@!\n");
    file("project/dependencies/test/fixture/v1.0.0/src/new/value", "\xff");
}

/** Both filesystem views choose the local version before seeing invalid external
 * bytes; external-only modules do not appear as holes before or after the move.
 */
static void it_preserves_whole_local_version_selection_in_the_projected_view(void) {
    setup("selected-version");
    package("project", "fixture");
    folder("project/packages/test/fixture/v1.0.0/src/old");
    file("project/packages/test/fixture/v1.0.0/src/old/value", "@!\n");
    external_version();
    ModuleRoot root = {.path = path("project"), .kind = MODULE_ROOT_PROJECT};
    RelocationRequest request = {&root, 1,
        path("project/packages/test/fixture/v1.0.0/src/old"),
        path("project/packages/test/fixture/v1.0.0/src/new"), NULL};
    assert(relocation_preview_build(&preview, &request) && preview.ready);
    assert(module("/test/fixture/v1.0.0/src/old/extra") == NULL);
    const RelocationModule *value = module("/test/fixture/v1.0.0/src/old/value");
    assert(value != NULL && strcmp(value->after_identity, "/test/fixture/v1.0.0/src/new/value") == 0);
    assert(preview.modules == value && value->next == NULL);
    untouched(&request);
    finish();
}

/** Losing a required selected local area is a preparation failure; the complete
 * external copy cannot supply it. Both failed catalogs remain safe to destroy.
 */
static void it_refuses_a_missing_projected_local_area_without_external_fallback(void) {
    setup("missing-selected-area");
    package("project", "fixture");
    file("project/packages/test/fixture/v1.0.0/src/value", "@!\n");
    external_version();
    ModuleRoot root = {.path = path("project"), .kind = MODULE_ROOT_PROJECT};
    RelocationRequest request = {&root, 1,
        path("project/packages/test/fixture/v1.0.0/tests"),
        path("project/packages/test/fixture/v1.0.0/old tests"), NULL};
    assert(!relocation_preview_build(&preview, &request) && !preview.ready);
    assert(preview.status == RELOCATION_INVALID_SOURCE);
    assert(strcmp(preview.path, preview.source) == 0);
    untouched(&request);
    finish();
}
