/** @file
 * @brief Inspect all intrinsic argument/result descriptors, ephemeral scalar selectors and invalid retired service forms.
 */
/** Method names/positions are fixed metadata and never synthetic module fields or action-dispatch parameters. */
static void it_specializes_all_six_method_interfaces_without_fabricated_declarations(void) {
    prepare_lists();
    SemanticStatus status = resolve_list("@!\nitems[]\n/test/fixture/v1.0.0/src/value\nsample\n/test/fixture/v1.0.0/src/value\n"
        "items[]\n append !\n  source& < & sample\n  element& > added&\n"
        " insert !\n  source& < & sample\n  position < |0\n  element& > inserted&\n"
        " remove !\n  <\n clear !\n swap !\n  <\n  < |0\n length !\n  > count\nappend\n sample\nclear\n sample\n");
    if (status != SEMANTIC_OK) { fprintf(stderr, "List interfaces: %s\n", semantic.error.message); }
    assert(status == SEMANTIC_OK && semantic.resolved);
    SemanticType *element = semantic_find_type(&semantic, "/test/fixture/v1.0.0/src/value");
    SemanticType *integer = semantic_find_type(&semantic, "//integer");
    assert(integer != NULL && integer == semantic.list_integer && integer->requires_size_bridge);
    const size_t inputs[] = {1, 2, 1, 0, 2, 0};
    const size_t outputs[] = {1, 1, 0, 0, 0, 1};
    for (size_t method = 0; method < SEMANTIC_LIST_METHOD_COUNT; method++) {
        SemanticStatement *statement = list_method((SemanticListMethod)method);
        assert(statement != NULL && statement->namespace_type == NULL && statement->access == NULL);
        assert(statement->list_interface.element == element && statement->list_interface.integer == integer);
        assert(!statement->list_interface.reference && statement->enclosing_operation == statement->list_service);
        assert(statement->list_service->operation_kind == SEMANTIC_LIST_SERVICE_OPERATION);
        size_t input_count = 0;
        size_t output_count = 0;
        for (SemanticBinding *binding = statement->bindings; binding != NULL; binding = binding->next) {
            bool output = binding->syntax->kind != AST_INPUT;
            assert(binding->field == NULL && binding->intrinsic != NULL && !binding->list);
            assert(binding->type == (binding->reference ? element : integer));
            size_t *count = output ? &output_count : &input_count;
            (*count)++;
        }
        assert(input_count == inputs[method] && output_count == outputs[method]);
    }
    assert(list_method(SEMANTIC_LIST_INSERT)->bindings->position == 1);
    assert(list_method(SEMANTIC_LIST_INSERT)->bindings->next->position == 0);
    destroy_semantic();
}
/** Append's handle and length's owned integer are ordinary selectable values with explicit ephemeral result ordinals. */
static void it_types_ephemeral_results_and_services_selected_from_module_results(void) {
    prepare_lists();
    file("generated/program/packages/test/fixture/v1.0.0/src/producer", "@>\nitems&[]\n/test/fixture/v1.0.0/src/resource\n@!\n");
    SemanticStatus status = resolve_list("@!\nproducer\n/test/fixture/v1.0.0/src/producer\nproducer\n items&[]\n  append !\n   element& !\n  length !\n   value !\n");
    assert(status == SEMANTIC_OK);
    SemanticStatement *append = list_method(SEMANTIC_LIST_APPEND);
    SemanticStatement *length = list_method(SEMANTIC_LIST_LENGTH);
    assert(append != NULL && length != NULL && append->list_interface.reference);
    SemanticStatement *element = append->actions->operation;
    SemanticStatement *value = length->actions->operation;
    assert(element->intrinsic_target && element->intrinsic_result == 0 && element->target_reference);
    assert(element->access == NULL && element->namespace_type == semantic_find_type(&semantic, "/test/fixture/v1.0.0/src/resource"));
    assert(value->intrinsic_target && value->intrinsic_result == 0 && !value->target_reference);
    assert(value->access == NULL && value->namespace_type == semantic.list_integer);
    assert(element->enclosing_operation == append && value->enclosing_operation == length);
    assert(append->list_service->enclosing_operation != NULL);
    destroy_semantic();
}
/** Arity, role/suffix/mode checks reject every implicit dispatcher and first-class-method reinterpretation. */
static void it_rejects_irrelevant_arguments_invalid_method_modes_and_retired_interfaces(void) {
    prepare_lists();
    const char *tails[] = {
        "items[] !\n", "items[]\n append >> copied\n",
        "items[]\n append !\n >> copied[]\n",
        "items[]\n unknown !\n", "items[]\n append %\n", "items[]\n append\n  >> result&\n",
        "items[]\n append& !\n", "items[]\n append[] !\n", "items[]\n length !\n  < |0\n",
        "items[]\n clear !\n  > result\n", "items[]\n remove !\n  source& < reference&\n",
        "items[]\n append !\n  source < reference&\n", "items[]\n append !\n  > result\n",
        "items[]\n swap !\n  <\n  <\n  <\n", "items[]\n append !\n  source& < reference&\n  source& < reference&\n",
        "items[]\n length !\n  value > result\n  value > result\n",
        "items[]\n length !\n  value& !\n", "items[]\n clear !\n  element& !\n",
        "items[]\n append !\n  element&[] !\n"
    };
    for (size_t index = 0; index < sizeof(tails) / sizeof(*tails); index++) {
        char text[512];
        int length = snprintf(text, sizeof(text), "@!\nitems[]\n/test/fixture/v1.0.0/src/value\nreference&\n/test/fixture/v1.0.0/src/value\nresult\n/test/fixture/v1.0.0/src/value\n%s", tails[index]);
        assert(length > 0 && (size_t)length < sizeof(text));
        expect_list(text, false);
    }
    /* Whole-list resume is already a structural syntax failure, before semantic profiles exist. */
    file("generated/program/packages/test/fixture/v1.0.0/src/main", "@!\nitems[]\n/test/fixture/v1.0.0/src/value\nitems[] %\n");
    setup_project();
    ProjectStatus loaded = load_root("generated/program");
    assert(loaded == PROJECT_SYNTAX_ERROR && !project.valid && diagnostic_valid(&project.error));
    destroy_project();
}
