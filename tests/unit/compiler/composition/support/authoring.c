/** @file
 * @brief Preserve an explicit native authoring overlay through full planning without weakening strict on-disk checking.
 */
/** Proposed ABI bytes have a separate source owner/input key and exactly match explicitly staged semantic/IR results. */
static void it_keeps_native_authoring_contracts_separate_from_strict_frontend_composition(void) {
    const char *proposal = "+ construct\n- finish\n";
    prepare_composed_files("@<\nvalue\n/test/fixture/v1.0.0/src/value\n@C\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/main.h", "#error Authoring must not compile native C\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/main.c", "#error Authoring must not execute native C\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/main.abi", "+ initialize\n+ duplicate\n- destroy\n");
    setup_frontend();
    ModuleRoot roots[] = {{"generated/project", NULL, 0, MODULE_ROOT_PROJECT},
        {"generated/standard", NULL, 0, MODULE_ROOT_STANDARD}};
    bool checked = compiler_frontend_authoring(&frontend, roots, 2, "/test/fixture/v1.0.0/src/main", proposal);
    assert(checked && frontend.valid && frontend.authoring && frontend.plan.valid);
    const SourceText *proposed = frontend.plan.entry->type->abi.bindings[ABI_DEFAULT].function.source;
    assert(proposed != frontend.project.entry->source->native_abi);
    assert(source_length(proposed) == strlen(proposal) && memcmp(source_bytes(proposed), proposal, strlen(proposal)) == 0);
    ProjectInput *input = project_inputs_find(&frontend.inputs, PROJECT_INPUT_CONFIGURATION, "native authoring ABI");
    assert(input != NULL && source_length(input->content) == strlen(proposal));
    assert(memcmp(source_bytes(input->content), proposal, strlen(proposal)) == 0);
    SemanticProgram staged;
    semantic_program_init(&staged, &frontend.project, sources.allocator);
    SemanticNativeAuthoring authoring = {frontend.project.entry, proposed};
    SemanticStatus status = staged_semantics(&staged, &frontend.specializations.closure, &authoring);
    assert(status == SEMANTIC_OK);
    compare_programs(&frontend.semantic, &staged);
    ProgramPlan planned;
    program_plan_init(&planned, &staged, sources.allocator);
    PlanStatus built = program_plan_build(&planned);
    assert(built == PLAN_OK);
    compare_plans(&frontend.plan, &planned);
    program_plan_destroy(&planned);
    semantic_program_destroy(&staged);
    CompilerFrontend strict;
    compiler_frontend_init(&strict, sources.allocator);
    checked = compiler_frontend_check(&strict, roots, 2, "/test/fixture/v1.0.0/src/main");
    assert(!checked && !strict.valid && strict.semantic_initialized && !strict.plan_initialized);
    assert(diagnostic_valid(&strict.error) && strstr(source_path(strict.error.primary.span.source), "main.abi") != NULL);
    compiler_frontend_destroy(&strict);
    assert(frontend.valid && frontend.plan.valid);
    destroy_frontend();
}
