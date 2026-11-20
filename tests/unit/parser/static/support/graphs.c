/**
 * @file
 * @brief Verify exclusive graph ownership, named source trees and exact comment regions.
 */

/** Empty graphs are complete metadata files, never ordinary implementation bodies. */
static void it_keeps_empty_static_graphs_separate_from_every_runtime_section(void) {
    const char *sources[] = {"@...", "@...\n", "# header\r\n@...\r\n# empty graph\r\n"};
    for (size_t index = 0; index < sizeof(sources) / sizeof(*sources); index++) {
        setup_tree();
        ParseStatus status = parse_text(sources[index]);
        assert(status == PARSE_OK && module.valid && module.sections.kind == MODULE_STATIC);
        assert(module.graph.first == NULL && module.body.first == NULL && module.constants.first == NULL);
        assert(module.parameters.first == NULL && module.results.first == NULL && module.warnings == NULL);
        assert(source_length(module.layout.source) == strlen(sources[index]));
        destroy_tree();
    }
}

/** Raw source distinguishes pair values from producer outputs without callee/category lookup. */
static void it_preserves_named_pairs_partial_values_and_multiple_producer_outputs(void) {
    const char *text = "@...\n.family for text\n/test/fixture/v1.0.0/src/dictionary\n .key type < //text\n"
        ".nested value\n.family for text\n .value type < /test/fixture/v1.0.0/src/box\n  .element type < //integer\n"
        "/test/fixture/v1.0.0/src/factory\n .input < .family for text\n .first > .one\n .second > .two\n";
    setup_tree();
    ParseStatus status = parse_text(text);
    assert(status == PARSE_OK && module.graph.first != NULL && module.body.first == NULL);
    const AstNode *first = module.graph.first, *second = first->next, *third = second->next;
    assert(first->kind == AST_STATIC_RESULT && second->kind == AST_STATIC_RESULT);
    assert(third->kind == AST_STATIC_PROJECTION && third->next == NULL && module.graph.last == third);
    assert(layout_text_is(first->as.static_result.name, ".family for text"));
    const AstTypeExpression *dictionary = first->as.static_result.expression;
    assert(dictionary->kind == AST_TYPE_MODULE_PATH && layout_text_is(dictionary->head, "/test/fixture/v1.0.0/src/dictionary"));
    assert(dictionary->selected_export.source == NULL && dictionary->first == dictionary->last);
    assert(layout_text_is(dictionary->first->label, ".key type"));
    assert(dictionary->first->kind == AST_STATIC_INPUT && layout_text_is(dictionary->first->input->head, "//text"));
    const AstTypeExpression *nested = second->as.static_result.expression;
    assert(nested->kind == AST_TYPE_PLACEHOLDER && layout_text_is(nested->head, ".family for text"));
    assert(layout_text_is(nested->first->input->head, "/test/fixture/v1.0.0/src/box"));
    assert(layout_text_is(nested->first->input->first->input->head, "//integer"));
    const AstTypeExpression *producer = third->as.static_projection;
    assert(layout_text_is(producer->head, "/test/fixture/v1.0.0/src/factory") && producer->first->kind == AST_STATIC_INPUT);
    const AstStaticBinding *one = producer->first->next, *two = one->next;
    assert(one->kind == AST_STATIC_OUTPUT && two->kind == AST_STATIC_OUTPUT && two == producer->last);
    assert(layout_text_is(one->label, ".first") && layout_text_is(one->receiver, ".one"));
    assert(layout_text_is(two->label, ".second") && layout_text_is(two->receiver, ".two"));
    assert(producer->span.end == two->span.end && third->span.end == two->span.end);
    assert(first->span.start < second->span.start && second->span.start < third->span.start);
    destroy_tree();
}

/** Heads open only their declared argument region; a dedented comment cannot reopen a child. */
static void it_obeys_graph_comment_regions_and_does_not_reopen_closed_children(void) {
    const char *valid[] = {
        "@...\n.type\n# same-level pair gap\n//integer\n # empty input region\n# module region\n",
        "@...\n/test/fixture/v1.0.0/src/factory\n # argument region\n .out > .result\n # output layout\n",
        "@...\n.value\n/test/fixture/v1.0.0/src/box\n .element < /test/fixture/v1.0.0/src/box\n  # nested argument region\n  .element < //integer\n"
    };
    for (size_t index = 0; index < sizeof(valid) / sizeof(*valid); index++) {
        setup_tree();
        ParseStatus status = parse_text(valid[index]);
        assert(status == PARSE_OK && module.valid);
        destroy_tree();
    }
    const char *invalid[] = {
        "@...\n # no child region at module level\n",
        "@...\n.value\n # a pair needs its same-level value\n//integer\n",
        "@...\n.value\n/test/fixture/v1.0.0/src/box\n  # skipped argument depth\n",
        "@...\n.value\n/test/fixture/v1.0.0/src/box\n# ended input region\n .element < //integer\n",
        "@...\n/test/fixture/v1.0.0/src/factory\n# ended producer block\n .out > .result\n"
    };
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        setup_tree();
        ParseStatus status = parse_text(invalid[index]);
        assert(status == PARSE_INVALID && !module.valid && diagnostic_valid(&module.error.diagnostic));
        destroy_tree();
    }
}
