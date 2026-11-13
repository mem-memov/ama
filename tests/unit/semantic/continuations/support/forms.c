/** @file
 * @brief Distinguish ordinary value/reference transfers from explicit scalar continuation copies and resume syntax.
 */
/** Copy syntax selects saved-position behavior explicitly, without introducing a continuation storage type. */
static void it_keeps_ordinary_handle_and_continuation_transfer_kinds_distinct(void) {
    prepare_types();
    SemanticStatus status = resolve_text("@!\nsource\n.value\n |% reference& # data  \nreference&\n.value\n &source\nordinary\n.value\n source\n"
        "continued\n.value\n % reference&\nelements[]\n.value\n % source\n"
        "ordinary\n source\nreference&\n &source\ncontinued\n % source\n%");
    assert(status == SEMANTIC_OK);
    assert(semantic.first->first->initializers->source->kind == AST_LITERAL_SOURCE);
    assert(layout_text_is(semantic.first->first->initializers->source->syntax->payload, "% reference& # data  "));
    size_t normal = 0;
    size_t handles = 0;
    size_t continuations = 0;
    for (const SemanticSymbol *field = semantic.first->first; field != NULL; field = field->next) {
        for (const SemanticInitializer *value = field->initializers; value != NULL; value = value->next) {
            if (value->transfer == TRANSFER_CLONE) { normal++; }
            if (value->transfer == TRANSFER_HANDLE) { handles++; }
            if (value->transfer == TRANSFER_CONTINUATION) { continuations++; }
        }
    }
    for (const SemanticStatement *statement = semantic.first->statements; statement != NULL; statement = statement->next) {
        if (statement->syntax->kind != AST_ASSIGNMENT) { continue; }
        if (statement->transfer == TRANSFER_CLONE) { normal++; }
        if (statement->transfer == TRANSFER_HANDLE) { handles++; }
        if (statement->transfer == TRANSFER_CONTINUATION) {
            continuations++;
            assert(statement->source->kind == AST_CONTINUATION_SOURCE);
        }
    }
    assert(normal == 2 && handles == 2 && continuations == 3);
    assert(semantic.first->last_statement->syntax->kind == AST_SUSPEND);
    destroy_semantic();
}
/** Scalar owned/reference sources produce owned continuation copies; reference/list destinations require other transfer forms. */
static void it_accepts_owned_scalar_continuation_sources_from_owned_and_reference_storage(void) {
    prepare_types();
    const char *forms[] = {"", "&", "[]", "&[]"};
    for (size_t source = 0; source < 2; source++) {
        for (size_t destination = 0; destination < 4; destination++) {
            char text[512];
            int length = snprintf(text, sizeof(text), "@!\nsource%s\n.value\ndestination%s\n.value\n"
                "destination%s\n %% source%s\n", forms[source], forms[destination], forms[destination], forms[source]);
            assert(length > 0 && (size_t)length < sizeof(text));
            expect_resolved(text, destination == 0);
        }
    }
    file("generated/program/worker", "@<\ninput\n.value\n@>\noutput\n.value\n@!\n");
    SemanticStatus status = resolve_text("@!\nsource\n.value\nreference&\n.value\n &source\nworker\n.worker\n"
        "worker !\n < % reference&\n > received\nworker %\n > resumed\nworker\n >> retained\n");
    assert(status == SEMANTIC_OK);
    SemanticStatement *entry = NULL;
    for (SemanticStatement *statement = semantic.first->statements; statement != NULL; statement = statement->next) {
        if (statement->syntax->kind == AST_CALL && statement->syntax->as.call.mode == AST_ENTRY_CALL) { entry = statement; break; }
    }
    assert(entry != NULL && entry->bindings->transfer == TRANSFER_CONTINUATION);
    assert(entry->bindings->source->reference && entry->bindings->next->transfer == TRANSFER_CLONE);
    assert(semantic.first->last_statement->bindings->transfer == TRANSFER_CLONE);
    destroy_semantic();
    expect_resolved("@!\nvalue\n.resource\nvalue\n % value\n", false);
}
/** Structural mode restrictions fail before execution, including empty resume inputs, whole-list copies, self source and immutable namespaces. */
static void it_rejects_resume_inputs_whole_list_sources_and_invalid_prefix_forms(void) {
    prepare_types();
    const char *invalid[] = {
        "worker %\n <\n", "worker %\n < value\n", "worker %\n >> value\n",
        "value\n % values[]\n", "value\n % references&[]\n", "value\n % &\n",
        "value\n % &value\n", "value\n % |literal\n", "value\n % .worker.constant\n",
        "value\n %value\n", "value\n % value\n:\n >\n"
    };
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        char text[512];
        int length = snprintf(text, sizeof(text), "@!\nworker\n.worker\nvalue\n.value\nvalues[]\n.value\nreferences&[]\n.value\n%s", invalid[index]);
        assert(length > 0 && (size_t)length < sizeof(text));
        file("generated/program/main", text);
        setup_project();
        ProjectStatus loaded = load_root("generated/program");
        assert(loaded == PROJECT_SYNTAX_ERROR && !project.valid && diagnostic_valid(&project.error));
        destroy_project();
    }
    expect_resolved("@=\nconstant\n.value\n@!\nvalue\n.value\nvalue\n % constant\n", false);
}
