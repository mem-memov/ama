/** @file
 * @brief Compose public frontend checking with independently owned explicit semantic/storage/control stages.
 */
/** Match every produced data/control domain while retaining two independently destructible semantic and plan owners. */
static void compare_plans(const ProgramPlan *left, const ProgramPlan *right) {
    assert(left != right && left->valid && right->valid && left->storage_ready && right->storage_ready);
    assert(left->next_selector == right->next_selector && left->status == PLAN_OK && right->status == PLAN_OK);
    assert(left->context_storage.domains == right->context_storage.domains);
    assert(left->context_storage.pools == right->context_storage.pools && left->context_storage.constants == right->context_storage.constants);
    assert(same_planned_type(left->entry, right->entry));
    const PlannedType *a = left->first, *b = right->first;
    for (; a != NULL && b != NULL; a = a->next, b = b->next) {
        compare_storage(a, b);
        compare_lifecycle(a, b, &a->create, &b->create);
        compare_lifecycle(a, b, &a->literal, &b->literal);
        compare_lifecycle(a, b, &a->destroy, &b->destroy);
        compare_lifecycle(a, b, &a->clone, &b->clone);
        compare_lifecycle(a, b, &a->continuation, &b->continuation);
        compare_controls(a, b);
    }
    assert(a == NULL && b == NULL);
    compare_symbol(left->constant_startup == NULL ? NULL : left->constant_startup->symbol,
        right->constant_startup == NULL ? NULL : right->constant_startup->symbol);
    compare_symbol(left->constant_cleanup == NULL ? NULL : left->constant_cleanup->symbol,
        right->constant_cleanup == NULL ? NULL : right->constant_cleanup->symbol);
    const SharedStorage *x = left->shared_first, *y = right->shared_first;
    for (; x != NULL && y != NULL; x = x->next, y = y->next) {
        assert(x != y && x->constant->ordinal == y->constant->ordinal && x->form == y->form);
        assert(x->immutable == y->immutable && x->service_storage == y->service_storage);
        assert(same_planned_type(x->value_type, y->value_type));
        compare_symbol(x->constant->symbol, y->constant->symbol);
        assert((x->previous == NULL ? 0 : x->previous->constant->ordinal + 1)
            == (y->previous == NULL ? 0 : y->previous->constant->ordinal + 1));
    }
    assert(x == NULL && y == NULL);
}
/** The authored scenario independently fixes domains, field forms, source payloads and absence of fake selector variables. */
static void assert_current_features(void) {
    const PlannedType *main = frontend.plan.entry;
    assert(main->field_count == 10 && main->frame_count == 8 && main->cell_count == 1 && main->origin_count == 1);
    assert(frontend.plan.context_storage.pools == 5 && frontend.plan.context_storage.constants == 2);
    assert(main->first->symbol->role == SYMBOL_PARAMETER && main->first->form == STORAGE_REFERENCE);
    assert(main->first->next->symbol->role == SYMBOL_RESULT && main->first->next->type == main);
    assert(main->first->next->symbol->initializers->source->kind == AST_SELF_REFERENCE_SOURCE);
    assert(main->cells->symbol->local && main->origins->symbol == main->cells->symbol);
    bool literal = false;
    for (const StorageField *field = main->first; field != NULL; field = field->next) {
        SourceSpan name = field->symbol->name;
        const unsigned char *bytes = source_bytes(name.source) + name.start;
        size_t size = name.end - name.start;
        assert(!(size == 6 && memcmp(bytes, "method", 6) == 0));
        assert(!(size == 4 && memcmp(bytes, "next", 4) == 0));
        if (size == 6 && memcmp(bytes, "source", 6) == 0) {
            SourceSpan payload = field->symbol->initializers->source->syntax->payload;
            const char *expected = "literal % ref& # bytes  ";
            assert(payload.end - payload.start == strlen(expected));
            assert(memcmp(source_bytes(payload.source) + payload.start, expected, strlen(expected)) == 0);
            literal = true;
        }
    }
    assert(literal && frontend.semantic.list_integer->requires_size_bridge);
    ProjectInput *header = project_inputs_find(&frontend.inputs, PROJECT_INPUT_FILE, "generated/standard/integer.h");
    const char *expected = "#error Frontend composition must not compile or execute native C\n";
    assert(header != NULL && source_length(header->content) == strlen(expected));
    assert(memcmp(source_bytes(header->content), expected, strlen(expected)) == 0);
    for (const ProjectInput *input = frontend.inputs.first; input != NULL; input = input->next) { assert(input->changed); }
}
/** Current constructs, empty bodies, reference cycles and immutable namespaces all preserve exact source metadata through the composite API. */
static void it_matches_complete_frontend_outputs_to_independent_explicit_stages(void) {
    const char *cases[] = {composed_source, "@!\n", "@!\nother&\n.main\nother& !\n", "@=\nvalue\n.value\n .palette.seed\n@!\n"};
    for (size_t shape = 0; shape < sizeof(cases) / sizeof(*cases); shape++) {
        prepare_composed_files(cases[shape]);
        setup_frontend();
        bool checked = check_composed();
        if (!checked) { fprintf(stderr, "Composite frontend: %s\n", frontend.error.message); }
        assert(checked && frontend.valid && frontend.plan.valid);
        assert(frontend.inputs.sealed && frontend.inputs.compared);
        if (shape == 0) { assert_current_features(); }
        SemanticProgram staged;
        semantic_program_init(&staged, &frontend.project, sources.allocator);
        SemanticStatus status = staged_semantics(&staged, NULL);
        assert(status == SEMANTIC_OK);
        compare_required_types(&staged);
        compare_programs(&frontend.semantic, &staged);
        ProgramPlan planned;
        program_plan_init(&planned, &staged, sources.allocator);
        PlanStatus result = program_plan_build(&planned);
        assert(result == PLAN_OK);
        compare_plans(&frontend.plan, &planned);
        assert(project_find(&frontend.project, ".unused draft") == NULL);
        program_plan_destroy(&planned);
        semantic_program_destroy(&staged);
        bool valid = program_plan_validate(&frontend.plan);
        assert(valid && frontend.plan.valid);
        destroy_frontend();
    }
}
