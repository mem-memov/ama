/** @file
 * @brief Refuse each frontend allocation through lexical collection and preserve diagnostic owners.
 */

/** Every actual check/authoring preparation allocation is refused once up to
 * the deterministic own-formal error. Each failure stays destructible with a
 * valid persistent diagnostic; no semantic or artifact preparation is reached.
 */
static void it_unwinds_every_new_frontend_static_admission_allocation_failure(void) {
    for (size_t authoring = 0; authoring < 2; authoring++) {
        size_t total = 0;
        for (size_t failure = 0; failure <= total; failure++) {
            prepare_native("@<\nunused\n...unused\n@C\n");
            setup_frontend();
            if (failure != 0) { failure_at = failure; }
            ModuleRoot root = {"generated/project", NULL, 0, MODULE_ROOT_PROJECT};
            bool accepted = authoring != 0 ? compiler_frontend_authoring(&frontend, &root, 1, ".main", "+ initialize\n- destroy\n")
                : compiler_frontend_check(&frontend, &root, 1, ".main");
            assert(!accepted && !frontend.valid && diagnostic_valid(&frontend.error));
            if (failure == 0) { total = allocation_count; assert(total != 0); rejected("...unused", ".main"); }
            else { assert(frontend.error.code == DIAGNOSTIC_OUT_OF_MEMORY); }
            assert(!frontend.semantic_initialized && !frontend.plan_initialized);
            destroy_frontend();
        }
    }
}
