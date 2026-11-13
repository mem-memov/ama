/** @file
 * @brief Fail every allocation in expanded dependency collection, native loading and requester diagnostics.
 */
/** Assemble a small closure crossing a folded header, traversal body, handler and native companion set. */
static void prepare_expanded_failures(bool malformed) {
    directory("generated/expanded failures");
    file("generated/expanded failures/main", "@!\nroot !\n < .native.value\n child !\n  condition\n  {\n   items[item&]\n   {\n    field\n    .body\n   }\n  }\n:\n field\n .handler\n");
    file("generated/expanded failures/native", "@C\n");
    file("generated/expanded failures/native.c", "/* body */\n");
    file("generated/expanded failures/native.h", "/* header */\n");
    file("generated/expanded failures/native.abi", malformed ? "\xff" : "+ initialize\n- destroy\n");
    file("generated/expanded failures/body", "@!\n");
    file("generated/expanded failures/handler", "@!\n");
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
