/** @file
 * @brief Supply exact result interfaces, native protocols and two roots for complete structured IR projects.
 */
/** Readable main source diagnostics accompany each real semantic-to-plan composition. */
static void prepare_structured_types(void) {
    prepare_types();
    file("generated/program/packages/test/fixture/v1.0.0/src/producer", "@<\ninput\n/test/fixture/v1.0.0/src/value\n@>\nleft\n/test/fixture/v1.0.0/src/worker\nright&\n/test/fixture/v1.0.0/src/worker\n@!\n");
    file("generated/program/packages/test/fixture/v1.0.0/src/worker", "@<\ninput\n/test/fixture/v1.0.0/src/value\n@>\nvalue\n/test/fixture/v1.0.0/src/value\ncondition\n/test/fixture/v1.0.0/src/value\nnext&\n/test/fixture/v1.0.0/src/worker\n@!\n%\n");
    file("generated/program/packages/test/fixture/v1.0.0/src/value.h", "#error Structured IR must not compile native C\n");
    directory("generated/standard");
    file("generated/standard/integer", "@C\n");
    file("generated/standard/integer.h", "#error Static plans must not inspect native bridge declarations\n");
    file("generated/standard/integer.c", "#error Structured IR must not execute native helpers\n");
    file("generated/standard/integer.abi", "+ initialize\n- destroy\n| literal\n= clone\n? condition\n[] index\n");
}
/** Compose a two-root complete plan, printing an authored fixture's exact failing source when needed. */
static void prepare_structured(const char *text) {
    prepare_structured_types();
    file("generated/program/packages/test/fixture/v1.0.0/src/main", text);
    setup_project();
    ModuleRoot roots[] = {{"generated/program", NULL, 0, MODULE_ROOT_PROJECT},
        {"generated/standard", NULL, 0, MODULE_ROOT_STANDARD}};
    ProjectStatus loaded = project_load(&project, roots, 2, "/test/fixture/v1.0.0/src/main");
    if (loaded != PROJECT_OK) { fprintf(stderr, "Structured source: %s\n%s\n", project.error.message, text); }
    assert(loaded == PROJECT_OK);
    semantic_program_init(&semantic, &project, sources.allocator);
    SemanticStatus status = semantic_fixture_check(&semantic);
    if (status != SEMANTIC_OK) { fprintf(stderr, "Structured semantics: %s\n%s\n", semantic.error.message, text); }
    assert(status == SEMANTIC_OK);
    program_plan_init(&plan, &semantic, sources.allocator);
    PlanStatus planned = program_plan_build(&plan);
    if (planned != PLAN_OK) { fprintf(stderr, "Structured plan: %s\n%s\n", plan.error.message, text); }
    assert(planned == PLAN_OK && plan.valid);
}
/** Find one exact operation-kind occurrence in allocation order; assertions inspect identities rather than assume label numbering. */
static ControlNode *structured_operation(ControlOperation operation, size_t occurrence) {
    for (ControlNode *node = plan.entry->controls; node != NULL; node = node->allocated_next) {
        if (node->operation == operation) {
            if (occurrence-- == 0) { return node; }
        }
    }
    assert(false);
    return NULL;
}
/** Match an exact resolved caller field by original spelling for lexical-environment assertions. */
static const SemanticSymbol *main_field(const char *name) {
    for (const SemanticSymbol *field = plan.entry->type->first; field != NULL; field = field->next) {
        if (layout_text_is(field->name, name)) { return field; }
    }
    return NULL;
}
