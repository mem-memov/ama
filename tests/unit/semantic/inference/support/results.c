/** @file
 * @brief Verify declared result types, omitted positions and lexical interleaving across selected actions.
 */
/** Each mode uses its exact declared result form; constant labels/member paths are not caller occurrences. */
static void it_types_output_receivers_from_entry_resume_retained_and_constant_interfaces(void) {
    prepare_inference();
    valid_inference("@!\nworker\n/test/fixture/v1.0.0/src/worker\nworker !\n > entry\n > entry reference&\n > entry values[]\n > entry references&[]\n"
        "worker %\n > resumed\nworker\n result >> retained\nworker !\n >\n > selected&\n"
        "/test/fixture/v1.0.0/src/palette\n tone >> tone\n tones[] >> colors[]\npalette\n /test/fixture/v1.0.0/src/palette/tone\n");
    const char *names[] = {"entry", "entry reference", "entry values", "entry references", "resumed", "retained", "selected", "tone", "colors", "palette"};
    const bool references[] = {false, true, false, true, false, false, true, false, false, false};
    const bool lists[] = {false, false, true, true, false, false, false, false, true, false};
    SemanticType *value_type = semantic_find_type(&semantic, "/test/fixture/v1.0.0/src/value");
    for (size_t index = 0; index < 10; index++) {
        SemanticSymbol *symbol = field(names[index]);
        assert(symbol != NULL && symbol->inferred && symbol->type == value_type);
        assert(symbol->reference == references[index] && symbol->list == lists[index]);
    }
    SemanticStatus resolved = semantic_resolve(&semantic);
    assert(resolved == SEMANTIC_OK);
    destroy_semantic();
}
/** A selected result can type a later operand; a later parent binding cannot retroactively type an earlier child input. */
static void it_orders_interleaved_selected_results_and_ordinary_bindings_by_written_position(void) {
    prepare_inference();
    file("generated/program/packages/test/fixture/v1.0.0/src/producer", "@>\nmethod\n/test/fixture/v1.0.0/src/worker\nlater\n/test/fixture/v1.0.0/src/worker\n@!\n");
    valid_inference("@!\nroot\n/test/fixture/v1.0.0/src/producer\nroot !\n method !\n  > first\n method > method\n later !\n  < first\n  > second\n");
    assert(field("first") != NULL && field("method") != NULL && field("second") != NULL);
    assert(field("first")->ordinal < field("method")->ordinal && field("method")->ordinal < field("second")->ordinal);
    assert(field("method")->type == semantic_find_type(&semantic, "/test/fixture/v1.0.0/src/worker"));
    assert(field("first")->type == field("second")->type);
    destroy_semantic();
    SemanticStatus status = infer_text("@!\nroot\n/test/fixture/v1.0.0/src/producer\nroot !\n later !\n  < not yet\n method > not yet\n");
    assert(status == SEMANTIC_INVALID && layout_text_is(semantic.error.primary.span, "not yet"));
    assert(field("not yet") == NULL);
    destroy_semantic();
}

/** Intrinsic service outputs have fixed typed interfaces, including direct-result list parents, without synthetic method fields. */
static void it_types_intrinsic_list_results_without_action_constants_or_method_instances(void) {
    prepare_inference();
    valid_inference("@!\nitems[]\n/test/fixture/v1.0.0/src/resource\nitems[]\n append !\n  element& > appended&\n insert !\n  <\n  <\n  > inserted&\n length !\n  value > count\n");
    assert(field("appended")->type == semantic_find_type(&semantic, "/test/fixture/v1.0.0/src/resource") && field("appended")->reference);
    assert(field("inserted")->type == field("appended")->type && field("inserted")->reference);
    assert(field("count")->type == semantic_find_type(&semantic, "//integer") && !field("count")->reference);
    assert(semantic_find_type(&semantic, "/test/fixture/v1.0.0/src/append") == NULL && field("append") == NULL && field("length") == NULL);
    assert(project_find(&project, "//integer") != NULL);
    destroy_semantic();
    file("generated/program/packages/test/fixture/v1.0.0/src/maker", "@>\nvalues[]\n/test/fixture/v1.0.0/src/value\n@!\n");
    valid_inference("@!\nproducer\n/test/fixture/v1.0.0/src/maker\nproducer !\n values[]\n  length !\n   > count\n");
    assert(field("count")->type == semantic_find_type(&semantic, "//integer"));
    assert(field("values") == NULL);
    destroy_semantic();
    const char *invalid[] = {
        "@!\nitems[]\n/test/fixture/v1.0.0/src/value\nitems[]\n append %\n",
        "@!\nitems[]\n/test/fixture/v1.0.0/src/value\nitems[]\n append\n  >> method\n",
        "@!\nitems[]\n/test/fixture/v1.0.0/src/value\nitems[]\n action !\n",
        "@!\nitems[]\n/test/fixture/v1.0.0/src/value\nitems[] !\n append !\n"
    };
    for (size_t index = 0; index < sizeof(invalid) / sizeof(invalid[0]); index++) {
        SemanticStatus status = infer_text(invalid[index]);
        assert(status == SEMANTIC_INVALID && !semantic.inference_valid);
        destroy_semantic();
    }
    file("generated/program/packages/test/fixture/v1.0.0/src/main", "@!\nitems[]\n/test/fixture/v1.0.0/src/value\nitems[]\n length !\n  > count\n");
    file("generated/program/packages/test/fixture/v1.0.0/src/integer", "@!\n");
    setup_project();
    ProjectStatus loaded = load_root("generated/program");
    assert(loaded == PROJECT_MISSING && strcmp(project.error.subject, "//integer") == 0);
    assert(layout_text_is(project.error.primary.span, "items[]"));
    destroy_project();
}
