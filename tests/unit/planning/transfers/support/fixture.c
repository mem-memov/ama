/** @file
 * @brief Supply ordered call interfaces and inspect replacement groups without running native code.
 */
static const char transfer_main[] = "@!\nsource\n.value\nsink\n.value\nworker\n.worker\nworker !\n second < source\n first < |literal % ref& # bytes  \n second result > sink\n first result > sink\n:\n";
/** Defaults deliberately read an earlier parameter; a noncopyable omitted reference proves no accidental source-copy path is inserted. */
static void prepare_transfers(void) {
    prepare_types();
    file("generated/program/worker", "@<\nfirst\n.value\nsecond\n.value\ndefaulted\n.value\n first\nexternal&\n.resource\n@>\nfirst result\n.value\n first\nsecond result\n.value\n second\n@!\nkept\n.value\n");
}
/** Compose the actual semantic API and plan boundary, retaining readable diagnostics for a failed authored fixture. */
static PlanStatus transfer_plan_text(const char *text) {
    file("generated/program/main", text);
    setup_project();
    ProjectStatus loaded = load_root("generated/program");
    if (loaded != PROJECT_OK) { fprintf(stderr, "Transfer source: %s\n%s\n", project.error.message, text); }
    assert(loaded == PROJECT_OK);
    semantic_program_init(&semantic, &project, sources.allocator);
    SemanticStatus status = semantic_fixture_check(&semantic);
    if (status != SEMANTIC_OK) { fprintf(stderr, "Transfer semantics: %s\n%s\n", semantic.error.message, text); }
    assert(status == SEMANTIC_OK);
    program_plan_init(&plan, &semantic, sources.allocator);
    return program_plan_build(&plan);
}
/** Walk one finite replacement group, asserting each explicit phase, its commit point and shared fatal route. */
static ControlNode *after_replacement(ControlNode *first, ReplacementContext context) {
    assert(first != NULL && first->replacement != NULL && first->replacement->first == first);
    const ReplacementPlan *recipe = first->replacement;
    assert(recipe->context == context);
    ControlNode *node = first->next;
    if (context == REPLACE_ASSIGNMENT) {
        assert(node->operation == CONTROL_SELECT_SOURCE && node->replacement == recipe);
        node = node->next;
    }
    const ControlOperation phases[] = {CONTROL_PREFLIGHT_REPLACEMENT, CONTROL_PREPARE_REPLACEMENT,
        CONTROL_COMMIT_REPLACEMENT, CONTROL_RELEASE_VALUES};
    for (size_t index = 0; index < sizeof(phases) / sizeof(*phases); index++) {
        assert(node != NULL && node->operation == phases[index] && node->replacement == recipe);
        assert(node->failure == first->failure && node->alternative == NULL && node->resume == NULL);
        if (phases[index] == CONTROL_COMMIT_REPLACEMENT) { assert(recipe->commit == node); }
        if (phases[index] == CONTROL_RELEASE_VALUES) { assert(recipe->last == node); }
        node = node->next;
    }
    return node;
}
/** Find an owned mutable phase for deliberate structural mutation; never cast away recipe endpoint constness. */
static ControlNode *phase_of(const ReplacementPlan *recipe, ControlOperation operation) {
    for (ControlNode *node = plan.entry->controls; node != NULL; node = node->allocated_next) {
        if (node->replacement == recipe && node->operation == operation) { return node; }
    }
    assert(false);
    return NULL;
}
