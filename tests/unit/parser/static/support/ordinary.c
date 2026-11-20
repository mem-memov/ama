/**
 * @file
 * @brief Verify simple ordinary Type pairs and the named static-export bridge syntax.
 */

/** Inputs/results remain variable declarations; export selection precedes ordinary literal payloads. */
static void it_keeps_simple_ordinary_type_pairs_and_export_initializers(void) {
    const char *selectors[] = {"@!", "@C"};
    for (size_t index = 0; index < 2; index++) {
        char text[256];
        int written = snprintf(text, sizeof(text),
            "@<\nargument\n.input type\n@>\nresult&[]\n/test/fixture/v1.0.0/src/assembly\n .selected type\n%s\n", selectors[index]);
        assert(written > 0 && (size_t)written < sizeof(text));
        setup_tree();
        ParseStatus status = parse_text(text);
        assert(status == PARSE_OK && module.graph.first == NULL);
        const AstNode *input = module.parameters.first, *output = module.results.first;
        assert(input && output && input->kind == AST_DECLARATION && output->kind == AST_DECLARATION);
        assert(layout_text_is(input->as.declaration.name->name, "argument"));
        assert(input->as.declaration.type->kind == AST_TYPE_PLACEHOLDER && input->as.declaration.type->first == NULL);
        assert(output->as.declaration.name->reference && output->as.declaration.name->whole_list);
        assert(layout_text_is(output->as.declaration.type->selected_export, ".selected type"));
        assert(output->as.declaration.type->first == NULL && output->as.declaration.first == NULL);
        destroy_tree();
    }
    setup_tree();
    ParseStatus status = parse_text("@!\nnumber\n/test/fixture/v1.0.0/src/assembly\n # selector region\n .integer type\n | raw # @...  \n");
    assert(status == PARSE_OK && module.body.first->kind == AST_DECLARATION);
    const AstNode *node = module.body.first;
    assert(layout_text_is(node->as.declaration.type->selected_export, ".integer type"));
    assert(node->as.declaration.initializer_count == 1);
    assert(layout_text_is(node->as.declaration.first->source->payload, " raw # @...  "));
    destroy_tree();
}

/** Concrete/placeholder namespaces retain ordinary >> requests; only concrete paths select an export. */
static void it_selects_constant_namespaces_without_ordinary_static_applications(void) {
    setup_tree();
    ParseStatus status = parse_text("@!\n/test/fixture/v1.0.0/src/assembly\n .ready type\n first >> one\n second >> two\n"
        ".visible type\n >\n");
    /* The second namespace deliberately uses a wrong runtime direction. */
    assert(status == PARSE_INVALID && !module.valid);
    destroy_tree();
    const char *valid[] = {"@!\n/test/fixture/v1.0.0/src/assembly\n .ready type\n first >> one\n second >> two\n",
        "@!\n.visible type\n first >> value\n", "@!\n//integer\n zero >> value\n"};
    for (size_t index = 0; index < sizeof(valid) / sizeof(*valid); index++) {
        setup_tree();
        status = parse_text(valid[index]);
        assert(status == PARSE_OK && module.body.first->kind == AST_CALL);
        const AstNode *read = module.body.first;
        assert(read->as.call.mode == AST_CONSTANT_READ && read->as.call.namespace_expression->first == NULL);
        assert(read->as.call.output_count == (index == 0 ? 2u : 1u));
        if (index == 0) { assert(layout_text_is(read->as.call.namespace_expression->selected_export, ".ready type")); }
        else { assert(read->as.call.namespace_expression->selected_export.source == NULL); }
        destroy_tree();
    }
}

/** Selectors have one level, no binding/suffix/children, and cannot attach to a placeholder. */
static void it_rejects_invalid_ordinary_selectors_and_nested_static_blocks(void) {
    const char *invalid[] = {
        "@!\nvalue\n/test/fixture/v1.0.0/src/assembly\n .one\n .two\n",
        "@!\nvalue\n/test/fixture/v1.0.0/src/assembly\n .one\n  .child\n",
        "@!\nvalue\n.type\n .selected\n",
        "@!\nvalue\n/test/fixture/v1.0.0/src/assembly\n .selected&\n",
        "@!\nvalue\n/test/fixture/v1.0.0/src/box\n .element < //integer\n",
        "@!\n/test/fixture/v1.0.0/src/assembly\n .out > .alias\n",
        "@!\n/test/fixture/v1.0.0/src/assembly\n .selected\n .key < //integer\n zero >> value\n",
        "@!\n/test/fixture/v1.0.0/src/assembly\n .selected\n .second\n zero >> value\n"
    };
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        setup_tree();
        ParseStatus status = parse_text(invalid[index]);
        assert(status == PARSE_INVALID && !module.valid && diagnostic_valid(&module.error.diagnostic));
        destroy_tree();
    }
}
