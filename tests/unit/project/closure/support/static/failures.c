/** @file
 * @brief Fail every allocation in static closure construction and its actual error paths.
 */

/** Baseline each exact preparation path, then inject every observed allocation
 * refusal. Missing and native-invalid cases must never publish partial success.
 */
static void it_unwinds_static_expression_closures_and_request_failures_at_every_allocation(void) {
    for (size_t scenario = 0; scenario < 3; scenario++) {
        if (scenario == 1) { prepare_missing_static_dependency(); }
        else {
            prepare_static_closure();
            if (scenario == 2) { file("generated/static standard/item-state.abi", "\xff"); }
        }
        setup_project();
        ProjectStatus status = load_static_closure(false);
        ProjectStatus expected[] = {PROJECT_OK, PROJECT_MISSING, PROJECT_CATALOG_ERROR};
        assert(status == expected[scenario]);
        if (scenario == 2) {
            assert(project.error.code == DIAGNOSTIC_INVALID_UTF8);
            assert(project.error.related_count == 1);
            assert(layout_text_is(project.error.related[0].location.span, "//item-state"));
        }
        size_t total = allocation_count;
        destroy_project();
        for (size_t failure = 1; failure <= total; failure++) {
            setup_project();
            failure_at = failure;
            assert(load_static_closure(false) == PROJECT_OUT_OF_MEMORY && !project.valid);
            assert(project.error.code == DIAGNOSTIC_OUT_OF_MEMORY);
            destroy_project();
        }
    }
}
