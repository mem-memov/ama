/** @file
 * @brief Preserve first-stage classifications/source diagnostics and release every complete-frontend allocation prefix.
 */
/** Active drafts, malformed sources/contracts, type/scope errors and constant cycles stop before publishing a plan; inactive drafts stay outside the closure. */
static void it_preserves_exact_failed_stage_diagnostics_and_invalid_project_ownership(void) {
    const char *invalid[] = {
        "@>\nunfinished\n.value\n",
        "@!\n\tvalue\n.value\n",
        "@!\nvalue\n.value\n",
        "@!\nvalue\n.value\nalias&\n.value\nvalue\n alias&\n",
        "@!\nitems[]\n.value\nitem&\n.other\nitems[item&]\n{\n}\n",
        "@=\nfirst\n.value\n .palette.seed\n@!\n",
        "@!\nvalue\n.value\n",
        "@=\nvalue\n.value\n |1\n"
    };
    for (size_t shape = 0; shape < sizeof(invalid) / sizeof(*invalid); shape++) {
        prepare_composed_files(invalid[shape]);
        file("generated/project/other", "@!\n");
        if (shape == 2) { file("generated/project/value.abi", "+ initialize\n+ repeated\n- destroy\n"); }
        if (shape == 5) { file("generated/project/palette", "@=\nseed\n.value\n .main.first\n"); }
        if (shape == 6) { int removed = unlink("generated/project/value.abi"); assert(removed == 0); }
        setup_frontend();
        bool checked = check_composed();
        assert(!checked && !frontend.valid && !frontend.plan_initialized && diagnostic_valid(&frontend.error));
        assert(source_span_valid(frontend.error.primary.span));
        if (shape < 2 || shape == 7) {
            assert(!frontend.project.valid && !frontend.semantic_initialized);
            assert(frontend.project.status == (shape == 0 ? PROJECT_DRAFT : shape == 7 ? PROJECT_INVALID_ENTRY : PROJECT_SYNTAX_ERROR));
        } else {
            assert(frontend.project.valid && frontend.specialization_initialized);
            CompilerSpecialization types;
            compiler_specialization_init(&types, &frontend.static_inventory, sources.allocator);
            const ProjectModule *roots[] = {frontend.project.entry};
            bool types_valid = compiler_specialization_check(&types, roots, 1);
            /* Storage-form and foreach-Type contradictions fail before ordinary
             * native/value/control checks; this independent owner uses the same
             * immutable Project, not the public frontend's derived records. */
            bool type_failure = shape == 3 || shape == 4;
            assert(types_valid == !type_failure);
            assert(frontend.specializations.valid == types_valid);
            if (type_failure) {
                assert(!frontend.semantic_initialized && !frontend.plan_initialized);
                compare_diagnostics(&frontend.error, &types.error);
                compiler_specialization_destroy(&types);
                destroy_frontend();
                continue;
            }
            assert(frontend.semantic_initialized && !frontend.semantic.constants_valid);
            SemanticProgram staged;
            semantic_program_init(&staged, &frontend.project, sources.allocator);
            SemanticStatus status = staged_semantics(&staged, NULL);
            assert(status == frontend.semantic.status && status == SEMANTIC_INVALID);
            compare_diagnostics(&frontend.error, &staged.error);
            assert(frontend.semantic.names_attempted == staged.names_attempted);
            assert(frontend.semantic.native_attempted == staged.native_attempted);
            assert(frontend.semantic.inference_attempted == staged.inference_attempted);
            assert(frontend.semantic.resolve_attempted == staged.resolve_attempted);
            assert(frontend.semantic.relations_attempted == staged.relations_attempted);
            assert(frontend.semantic.constants_attempted == staged.constants_attempted);
            semantic_program_destroy(&staged);
            compiler_specialization_destroy(&types);
        }
        destroy_frontend();
    }
}
/** Fail each counted allocation in real loading, input capture, semantic analysis and complete structured plan construction. */
static void it_unwinds_every_complete_frontend_source_semantic_and_plan_allocation(void) {
    prepare_composed_files(composed_source);
    setup_frontend();
    bool checked = check_composed();
    assert(checked);
    size_t count = allocation_count;
    destroy_frontend();
    bool failed_specialization = false;
    bool failed_semantic = false;
    bool failed_plan = false;
    for (size_t failure = 1; failure <= count; failure++) {
        setup_frontend();
        failure_at = failure;
        checked = check_composed();
        assert(!checked && !frontend.valid && frontend.error.code == DIAGNOSTIC_OUT_OF_MEMORY);
        assert(diagnostic_valid(&frontend.error));
        failed_specialization |= frontend.specialization_initialized && !frontend.semantic_initialized;
        failed_semantic |= frontend.semantic_initialized;
        failed_plan |= frontend.plan_initialized;
        destroy_frontend();
    }
    assert(failed_specialization && failed_semantic && failed_plan);
}
