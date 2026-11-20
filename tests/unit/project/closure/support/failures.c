/** @file
 * @brief Fail every allocation in expanded dependency collection, native loading and requester diagnostics.
 */
/** Assemble a small closure crossing a folded header, traversal body, handler and native companion set. */
static void prepare_expanded_failures(bool malformed) {
    fixture_package_prepare("generated/expanded failures", "fixture");
    file("generated/expanded failures/packages/test/fixture/v1.0.0/src/main", "@!\nroot !\n < /test/fixture/v1.0.0/src/native/value\n child !\n  condition\n  {\n   items[item&]\n   {\n    field\n    /test/fixture/v1.0.0/src/body\n   }\n  }\n:\n field\n /test/fixture/v1.0.0/src/handler\n");
    file("generated/expanded failures/packages/test/fixture/v1.0.0/src/native", "@C\n");
    file("generated/expanded failures/packages/test/fixture/v1.0.0/src/native.c", "/* body */\n");
    file("generated/expanded failures/packages/test/fixture/v1.0.0/src/native.h", "/* header */\n");
    file("generated/expanded failures/packages/test/fixture/v1.0.0/src/native.abi", malformed ? "\xff" : "+ initialize\n- destroy\n");
    file("generated/expanded failures/packages/test/fixture/v1.0.0/src/body", "@!\n");
    file("generated/expanded failures/packages/test/fixture/v1.0.0/src/handler", "@!\n");
}
/** Each successful/error prefix remains safely owned and every actual allocation failure reports OOM rather than partial closure success. */
static void it_unwinds_every_expanded_discovery_and_native_error_allocation_failure(void) {
    for (size_t malformed = 0; malformed < 2; malformed++) {
        prepare_expanded_failures(malformed != 0);
        setup_project();
        ProjectStatus status = load_root("generated/expanded failures");
        assert(status == (malformed ? PROJECT_CATALOG_ERROR : PROJECT_OK));
        size_t total = allocation_count;
        destroy_project();
        for (size_t failure = 1; failure <= total; failure++) {
            setup_project();
            failure_at = failure;
            status = load_root("generated/expanded failures");
            assert(status == PROJECT_OUT_OF_MEMORY && !project.valid);
            assert(project.error.code == DIAGNOSTIC_OUT_OF_MEMORY);
            destroy_project();
        }
    }
}
