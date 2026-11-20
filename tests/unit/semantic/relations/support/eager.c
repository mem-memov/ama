/** @file
 * @brief Check exact eager-edge classification and inferred defaults independently from clone ownership edges.
 */
/** Exercise every field role/form/initializer/inference combination, including nonowning and shared exclusions. */
static void it_classifies_every_eager_field_role_form_and_initializer_state(void) {
    for (unsigned bits = 0; bits < 128; bits++) {
        setup_test();
        ModuleSource module_source = {.logical_path = "/test/fixture/v1.0.0/src/main"};
        ProjectModule module = {.source = &module_source};
        SemanticInitializer initial = {0};
        SemanticType type = {.module = &module, .executable = true};
        SemanticSymbol field = {.owner = &type, .type = &type,
            .role = (SymbolRole)(bits & 3u), .reference = (bits & 4u) != 0,
            .list = (bits & 8u) != 0, .inferred = (bits & 16u) != 0,
            .initializers = (bits & 32u) != 0 ? &initial : NULL};
        type.first = &field;
        SemanticProgram program = {.first = &type};
        /* Half the cases use Ama payloads; the other half explicitly seed native copying. */
        module.syntax.sections.kind = (bits & 64u) ? MODULE_NATIVE : MODULE_AMA;
        type.abi.bindings[ABI_CLONE].present = true;
        dependency_graph_init(&program.eager, sources.allocator);
        bool built = semantic_eager_graph(&program);
        assert(built && program.eager.first != NULL && program.eager.first->next == NULL);
        bool expected = field.role != SYMBOL_CONSTANT && !field.reference
            && (!field.list || field.initializers != NULL);
        assert((program.eager.first->first != NULL) == expected);
        GraphStatus status = dependency_order(&program.eager);
        assert(status == (expected ? GRAPH_CYCLE : GRAPH_OK));
        semantic_clone_capabilities(&program);
        assert(type.cloneable);
        dependency_graph_destroy(&program.eager);
        destroy_test();
    }
}
/** Inference adds ordinary eager owned fields, while inferred empty lists stay finite and preserve static element capability. */
static void it_keeps_inferred_default_storage_in_the_correct_relations(void) {
    prepare_types();
    const char *valid[] = {
        "@!\nsource[]\n/test/fixture/v1.0.0/src/main\ncopy[]\n source[]\n",
        "@!\nsource&\n/test/fixture/v1.0.0/src/main\ncopy&\n source&\n",
        "@!\nsource&[]\n/test/fixture/v1.0.0/src/resource\ncopy&[]\n source&[]\n",
        "@!\nsource\n/test/fixture/v1.0.0/src/value\ncopy\n source\n"
    };
    for (size_t index = 0; index < sizeof(valid) / sizeof(*valid); index++) {
        SemanticStatus status = resolve_text(valid[index]);
        assert(status == SEMANTIC_OK);
        SemanticSymbol *copy = semantic_entry()->last;
        assert(copy->inferred && copy->initializers == NULL);
        status = semantic_relations(&semantic);
        assert(status == SEMANTIC_OK && semantic.relations_valid);
        destroy_semantic();
    }
    /* The empty list has no eager edge, but still prevents owned cloning of its element type. */
    SemanticStatus status = resolve_text("@!\nsource[]\n/test/fixture/v1.0.0/src/resource\ncopy[]\n source[]\n");
    assert(status == SEMANTIC_INVALID && strstr(semantic.error.message, "independent copying") != NULL);
    destroy_semantic();
}
