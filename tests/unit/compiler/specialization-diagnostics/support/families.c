/** @file
 * @brief Require original interface and equation provenance across static error families.
 * @see [T189 contract](../../../../../docs/implementation/specialization-diagnostics.md)
 */

/** Unknown foreign formals and wrong argument categories must retain both the actual application and original constructor interface. */
static void it_reports_static_signature_and_kind_conflicts_with_original_constructor_context(void) {
    for (size_t shape = 0; shape < 2; shape++) {
        directory("generated/project");
        file("generated/project/main", "@!\nvalue\n.factory\n ...out\n");
        file("generated/project/target", "@<\ninput\n...slot\n@!\n");
        file("generated/project/factory", shape
            ? "@...\n...out\n.target\n ...slot < .target\n"
            : "@...\n...out\n.target\n ...unknown < ..integer\n");
        setup_frontend();
        ModuleRoot roots[] = {{"generated/project", NULL, 0, MODULE_ROOT_PROJECT},
            {TEST_STANDARD_ROOT, NULL, 0, MODULE_ROOT_STANDARD}};
        assert(!compiler_frontend_check(&frontend, roots, 2, ".main"));
        const Diagnostic *error = &frontend.error;
        assert(diagnostic_valid(error) && !frontend.valid && !frontend.plan_initialized);
        assert(diagnostic_render(stderr, error) == DIAGNOSTIC_RENDER_OK);
        assert(references_site(error, project_find(&frontend.project, ".factory")->source->text, 3, 4));
        assert(references_site(error, project_find(&frontend.project, ".target")->source->text, 1, SIZE_MAX));
        destroy_frontend();
    }
}

/** Same-key nominal recursion may close; infinite ownership and cyclic shared values must instead report their actual defining edges. */
static void it_distinguishes_original_eager_ownership_and_shared_value_cycle_locations(void) {
    for (size_t shape = 0; shape < 2; shape++) {
        directory("generated/project");
        file("generated/project/main", shape
            ? "@=\nvalue\n..integer\n .factory.value\n@!\n"
            : "@!\nchild\n.main\n");
        if (shape) { file("generated/project/factory", "@=\nvalue\n..integer\n .main.value\n"); }
        setup_frontend();
        ModuleRoot roots[] = {{"generated/project", NULL, 0, MODULE_ROOT_PROJECT},
            {TEST_STANDARD_ROOT, NULL, 0, MODULE_ROOT_STANDARD}};
        assert(!compiler_frontend_check(&frontend, roots, 2, ".main"));
        const Diagnostic *error = &frontend.error;
        assert(diagnostic_valid(error) && !frontend.valid && !frontend.plan_initialized);
        assert(frontend.specializations.closure.closed);
        assert(diagnostic_render(stderr, error) == DIAGNOSTIC_RENDER_OK);
        assert(references_site(error, frontend.project.entry->source->text, 2, 4));
        if (shape) { assert(references_site(error, project_find(&frontend.project, ".factory")->source->text, 2, 4)); }
        destroy_frontend();
    }
}

/** A returned family retains an extra actual residual, and its refusal as an ordinary Type must point to both the export and that residual declaration. */
static void it_preserves_extra_template_residuals_and_reports_their_original_formals(void) {
    directory("generated/project");
    file("generated/project/main", "@!\nvalue\n.factory\n ...out\n");
    file("generated/project/target", "@<\nkey\n...key\nvalue\n...value\nallocator\n...allocator\n@!\n");
    file("generated/project/factory", "@...\n...out\n.target\n ...key < ..integer\n ...value < ..text\n");
    setup_frontend();
    ModuleRoot roots[] = {{"generated/project", NULL, 0, MODULE_ROOT_PROJECT},
        {TEST_STANDARD_ROOT, NULL, 0, MODULE_ROOT_STANDARD}};
    assert(!compiler_frontend_check(&frontend, roots, 2, ".main"));
    const Diagnostic *error = &frontend.error;
    assert(diagnostic_valid(error) && !frontend.valid && !frontend.plan_initialized);
    assert(diagnostic_render(stderr, error) == DIAGNOSTIC_RENDER_OK);
    assert(references_site(error, frontend.project.entry->source->text, 4, 4));
    assert(references_site(error, project_find(&frontend.project, ".factory")->source->text, 2, 2));
    assert(references_site(error, project_find(&frontend.project, ".target")->source->text, 7, 7));
    destroy_frontend();
}

/** A separately complete producer may warm caches, but cannot supply an unrelated
 * producer's missing inputs; ordinary call inference cannot cross that boundary. */
static void it_preserves_independent_bridge_refusals_across_declaration_and_root_orders(void) {
    for (size_t order = 0; order < 2; order++) {
        directory("generated/project");
        file("generated/project/main", order
            ? "@!\nvalue\n.factory\n ...out\nknown\n.identity\n ...out\n"
            : "@!\nknown\n.identity\n ...out\nvalue\n.factory\n ...out\n");
        file("generated/project/identity", "@...\n...out\n..integer\n");
        file("generated/project/factory", "@...\n...out\n..integer\n...other\n...missing\n");
        setup_frontend();
        ModuleRoot roots[] = {{"generated/project", NULL, 0, MODULE_ROOT_PROJECT},
            {TEST_STANDARD_ROOT, NULL, 0, MODULE_ROOT_STANDARD}};
        if (order) { ModuleRoot swap = roots[0]; roots[0] = roots[1]; roots[1] = swap; }
        assert(!compiler_frontend_check(&frontend, roots, 2, ".main"));
        const Diagnostic *error = &frontend.error;
        assert(diagnostic_valid(error) && !frontend.valid && !frontend.plan_initialized);
        assert(diagnostic_render(stderr, error) == DIAGNOSTIC_RENDER_OK);
        assert(references_site(error, project_find(&frontend.project, ".factory")->source->text, 1, 5));
        assert(references_site(error, frontend.project.entry->source->text, order ? 4 : 7, order ? 4 : 7));
        destroy_frontend();
    }
}

/** Reject malformed internal joins at their original lexical line before any
 * specialization is published, rather than searching for a repaired module. */
static void it_rejects_malformed_static_names_at_the_original_source_line(void) {
    const char *names[] = {"...type--name", "...type.", "...type..name"};
    for (size_t index = 0; index < sizeof(names) / sizeof(names[0]); index++) {
        directory("generated/project");
        file("generated/project/main", "@!\nvalue\n.factory\n ...out\n");
        char text[128];
        int length = snprintf(text, sizeof(text), "@...\n...out\n%s\n", names[index]);
        assert(length > 0 && (size_t)length < sizeof(text));
        file("generated/project/factory", text);
        setup_frontend();
        ModuleRoot root = {"generated/project", NULL, 0, MODULE_ROOT_PROJECT};
        assert(!compiler_frontend_check(&frontend, &root, 1, ".main"));
        assert(diagnostic_valid(&frontend.error) && !frontend.valid && !frontend.plan_initialized);
        assert(diagnostic_render(stderr, &frontend.error) == DIAGNOSTIC_RENDER_OK);
        assert(frontend.error.primary.span.source &&
            strstr(source_path(frontend.error.primary.span.source), "factory"));
        SourcePosition position;
        assert(source_position(frontend.error.primary.span.source, frontend.error.primary.span.start, &position));
        assert(position.line == 3);
        destroy_frontend();
    }
}
