/** @file
 * @brief Verify source-derived native policy on strict, authoring, catalog and explicit-required routes.
 */

/** Definition categories are checked before invalid companion ABI bytes and
 * explicit proposed authoring ABI could hide or specialize their own formals.
 */
static void it_rejects_native_own_formals_before_check_capture_or_authoring_abi_proposals(void) {
    const char *sources_text[] = {
        "@<\ninput\n...input type\n@C\n",
        "@>\noutput\n...result type\n@C\n"
    };
    const char *names[] = {"...input type", "...result type"};
    for (size_t index = 0; index < sizeof(sources_text) / sizeof(*sources_text); index++) {
        for (size_t authoring = 0; authoring < 2; authoring++) {
            prepare_native(sources_text[index]);
            setup_frontend();
            ModuleRoot root = {"generated/project", NULL, 0, MODULE_ROOT_PROJECT};
            bool accepted = authoring != 0 ? compiler_frontend_authoring(&frontend, &root, 1, ".main", "invalid proposed ABI\n")
                : compiler_frontend_check(&frontend, &root, 1, ".main");
            assert(!accepted);
            rejected(names[index], ".main");
            destroy_frontend();
        }
    }
}

/** These entry points bypass ordinary single-entry loading; both must perform
 * the same policy before capturing inputs or accepting the loaded definition.
 */
static void it_checks_native_admission_on_preloaded_catalog_and_explicit_required_paths(void) {
    prepare_native("@<\nunused\n...unused\n@C\n");
    setup_frontend();
    ModuleRoot root = {"generated/project", NULL, 0, MODULE_ROOT_PROJECT};
    assert(module_catalog_build(&frontend.project.catalog, &root, 1) == MODULE_OK);
    assert(!compiler_frontend_check_catalog(&frontend, &root, 1, ".main", NULL, 0));
    rejected("...unused", ".main");
    destroy_frontend();
    directory("generated/required");
    file("generated/required/main", "@!\n");
    file("generated/required/bad", "@<\nunused\n...required unused\n@C\n");
    setup_frontend();
    ModuleRoot required_root = {"generated/required", NULL, 0, MODULE_ROOT_PROJECT};
    const char *required[] = {".bad"};
    assert(!compiler_frontend_load_required(&frontend, &required_root, 1, ".main", required, 1));
    rejected("...required unused", ".bad");
    destroy_frontend();
}

/** Concrete native behavior remains the ordinary strict/explicit-authoring
 * path; invalid C text proves the frontend performs no native C compilation.
 */
static void it_preserves_existing_concrete_native_check_and_authoring_paths(void) {
    for (size_t authoring = 0; authoring < 2; authoring++) {
        prepare_native("@C\n");
        file("generated/project/main.abi", "+ initialize\n- destroy\n");
        setup_frontend();
        ModuleRoot root = {"generated/project", NULL, 0, MODULE_ROOT_PROJECT};
        bool accepted = authoring != 0 ? compiler_frontend_authoring(&frontend, &root, 1, ".main", "+ initialize\n- destroy\n")
            : compiler_frontend_check(&frontend, &root, 1, ".main");
        assert(accepted && frontend.valid && frontend.static_inventory.valid && frontend.static_initialized);
        assert(frontend.semantic_initialized && frontend.plan_initialized && frontend.inputs.sealed);
        assert(frontend.static_inventory.first->parameter_count == 0);
        destroy_frontend();
    }
}
