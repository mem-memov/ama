/** @file
 * @brief Refuse conflicting delivered names, missing required source families and incomplete ordinary selections.
 * @see [T191 contract](../../../../../docs/implementation/template-source-delivery.md)
 */

/** Import roots have no priority fallback: duplicate full module identities
 * refuse before specialization, in either configured root order. */
static void it_refuses_conflicting_library_names_in_both_root_orders(void) {
    for (size_t reverse = 0; reverse < 2; reverse++) {
        prepare_delivery_sources();
        fixture_package_prepare("generated/conflict", "library");
        directory("generated/conflict/packages/test/library/v1.0.0/src/lib");
        file("generated/conflict/packages/test/library/v1.0.0/src/lib/forward", "@!\n");
        setup_frontend();
        ModuleRoot roots[] = {{"generated/project", NULL, 0, MODULE_ROOT_PROJECT},
            {"generated/library", NULL, 0, MODULE_ROOT_PROJECT},
            {"generated/conflict", NULL, 0, MODULE_ROOT_PROJECT},
            {TEST_STANDARD_ROOT, NULL, 0, MODULE_ROOT_STANDARD}};
        if (reverse) { ModuleRoot swap = roots[1]; roots[1] = roots[2]; roots[2] = swap; }
        assert(!compiler_frontend_check(&frontend, roots, 4, "/test/fixture/v1.0.0/src/main"));
        assert(frontend.error.code == DIAGNOSTIC_AMBIGUOUS_MODULE && diagnostic_valid(&frontend.error));
        assert(!frontend.valid && !frontend.plan_initialized && !frontend.inputs.sealed);
        assert(diagnostic_render(stderr, &frontend.error) == DIAGNOSTIC_RENDER_OK);
        destroy_frontend();
    }
}

/** Missing project modules and missing standard-family modules retain their
 * actual requiring source; available names in the other family are irrelevant. */
static void it_refuses_missing_required_library_sources_without_namespace_fallback(void) {
    for (size_t standard = 0; standard < 2; standard++) {
        prepare_delivery_sources();
        if (standard) {
            file("generated/library/packages/test/library/v1.0.0/src/lib/assembly", "@...\n.result\n/test/library/v1.0.0/src/lib/forward\n .slot < //lib/value\n");
        } else { assert(unlink("generated/library/packages/test/library/v1.0.0/src/lib/forward") == 0); }
        setup_frontend();
        ModuleRoot roots[] = {{"generated/project", NULL, 0, MODULE_ROOT_PROJECT},
            {"generated/library", NULL, 0, MODULE_ROOT_PROJECT},
            {TEST_STANDARD_ROOT, NULL, 0, MODULE_ROOT_STANDARD}};
        assert(!compiler_frontend_check(&frontend, roots, 3, "/test/fixture/v1.0.0/src/main"));
        assert(frontend.error.code == DIAGNOSTIC_MISSING_MODULE && diagnostic_valid(&frontend.error));
        assert(frontend.error.subject && strcmp(frontend.error.subject, standard ? "//lib/value" : "/test/library/v1.0.0/src/lib/forward") == 0);
        assert(frontend.error.primary.span.source);
        assert(strstr(source_path(frontend.error.primary.span.source), "library/packages/test/library/v1.0.0/src/lib/assembly"));
        assert(!frontend.valid && !frontend.plan_initialized && !frontend.inputs.sealed);
        assert(diagnostic_render(stderr, &frontend.error) == DIAGNOSTIC_RENDER_OK);
        destroy_frontend();
    }
}

/** A delivered open constructor is source metadata, never an implicit concrete
 * instance or native-erased runtime object when its binding is omitted. */
static void it_refuses_unclosed_delivered_templates_at_the_ordinary_selection(void) {
    prepare_delivery_sources();
    file("generated/library/packages/test/library/v1.0.0/src/lib/assembly", "@...\n.result\n/test/library/v1.0.0/src/lib/forward\n");
    setup_frontend();
    ModuleRoot roots[] = {{"generated/project", NULL, 0, MODULE_ROOT_PROJECT},
        {"generated/library", NULL, 0, MODULE_ROOT_PROJECT},
        {TEST_STANDARD_ROOT, NULL, 0, MODULE_ROOT_STANDARD}};
    assert(!compiler_frontend_check(&frontend, roots, 3, "/test/fixture/v1.0.0/src/main"));
    assert(diagnostic_valid(&frontend.error) && frontend.error.code == DIAGNOSTIC_TYPE_MISMATCH);
    assert(!frontend.valid && !frontend.plan_initialized);
    assert(diagnostic_render(stderr, &frontend.error) == DIAGNOSTIC_RENDER_OK);
    destroy_frontend();
}
