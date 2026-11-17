/** @file
 * @brief Check structural static bindings, finite nesting and expression failure ownership.
 */
/** Own source/layout before a direct component attempt; the returned cursor
 * borrows the current AST fixture, which the caller destroys after inspection.
 */
static Parser static_fixture(const char *text) {
    const SourceText *source;
    assert(source_manager_add(&sources, "static-expression",
        (const unsigned char *)text, strlen(text), &source) == SOURCE_OK);
    assert(layout_read(&module.layout, source) == LAYOUT_OK);
    return (Parser){.module = &module, .line = module.layout.first};
}

/** Nested RHS expressions own only their indented bindings, preserving sibling
 * order and leaving an ordinary initializer for the declaration consumer.
 */
static void it_preserves_nested_expression_owners_and_initializer_boundaries(void) {
    setup_tree();
    Parser parser = static_fixture(".outer\n ...factory < .dictionary\n"
        "  ...key type < ..text\n ...element < ...local type\n"
        " ...result > ...received\n source-name\n");
    AstTypeExpression *expression = NULL;
    assert(parser_type_expression(&parser, parser.line->code, 0, &expression));
    assert(expression->kind == AST_TYPE_MODULE_PATH && layout_text_is(expression->head, ".outer"));
    AstStaticBinding *factory = expression->first;
    assert(factory != NULL && layout_text_is(factory->label, "...factory"));
    assert(layout_text_is(factory->input->head, ".dictionary"));
    assert(layout_text_is(factory->input->first->label, "...key type"));
    assert(layout_text_is(factory->input->first->input->head, "..text"));
    assert(factory->input->first->next == NULL && factory->span.end == factory->input->span.end);
    AstStaticBinding *element = factory->next;
    assert(element != NULL && element->input->kind == AST_TYPE_PLACEHOLDER);
    assert(layout_text_is(element->input->head, "...local type"));
    AstStaticBinding *result = element->next;
    assert(result == expression->last && result->next == NULL);
    assert(result->kind == AST_STATIC_OUTPUT && layout_text_is(result->receiver, "...received"));
    assert(expression->span.end == result->span.end);
    assert(parser.line != NULL && layout_text_is(parser.line->code, "source-name"));
    destroy_tree();
}

/** Static skips retain source positions without fabricating input expressions
 * or output aliases; their two counters will be resolved in a later component.
 */
static void it_retains_positional_input_and_output_skips_as_distinct_bindings(void) {
    setup_tree();
    Parser parser = static_fixture(".factory\n ... <\n ... < ..integer\n"
        " ... >\n ... > ...alias\n");
    AstTypeExpression *expression = NULL;
    assert(parser_type_expression(&parser, parser.line->code, 0, &expression));
    AstStaticBinding *binding = expression->first;
    assert(binding->kind == AST_STATIC_INPUT && binding->positional && binding->omitted);
    assert(binding->input == NULL);
    binding = binding->next;
    assert(binding->kind == AST_STATIC_INPUT && binding->positional && !binding->omitted);
    assert(layout_text_is(binding->input->head, "..integer"));
    binding = binding->next;
    assert(binding->kind == AST_STATIC_OUTPUT && binding->positional && binding->omitted);
    assert(binding->receiver.source == NULL);
    binding = binding->next;
    assert(binding == expression->last && binding->kind == AST_STATIC_OUTPUT);
    assert(binding->positional && !binding->omitted && layout_text_is(binding->receiver, "...alias"));
    assert(parser.line == NULL);
    destroy_tree();
}

/** Invalid binding syntax never publishes a partial root, even after earlier
 * bindings allocated nodes; all those nodes remain with the AST cleanup owner.
 */
static void it_rejects_static_operand_order_and_indentation_errors_without_publication(void) {
    const char *invalid[] = {
        "...\n", ".module\n ...key <\n", ".module\n ...result >\n",
        ".module\n ... > receiver\n", ".module\n ... <\n  ...inner < .type\n",
        ".module\n ... > ...alias\n ... < .type\n", ".module\n  ... < .type\n",
        ".module\n ...x < .type[]\n", ".module\n ...x < ....type\n",
        ".module\n ...x < |literal\n", ".module\n ... > ...alias\n  ...x < .type\n"
    };
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        setup_tree();
        Parser parser = static_fixture(invalid[index]);
        AstTypeExpression sentinel = {0};
        AstTypeExpression *expression = &sentinel;
        assert(!parser_type_expression(&parser, parser.line->code, 0, &expression));
        assert(parser.failed && expression == &sentinel && diagnostic_valid(&module.error.diagnostic));
        destroy_tree();
    }
}

/** Construct a bounded test source with an exactly sized triangular indentation
 * region. The test bound limits this fixture generator, never the parser.
 */
static char *nested_static_source(size_t depth) {
    assert(depth <= 600);
    const char *head = ".node\n";
    const char *binding = "... < .node\n";
    size_t head_length = strlen(head), binding_length = strlen(binding);
    size_t size = head_length + depth * (depth + 1) / 2 + depth * binding_length + 1;
    char *text = malloc(size);
    assert(text != NULL);
    memcpy(text, head, head_length);
    size_t used = head_length;
    for (size_t indentation = 1; indentation <= depth; indentation++) {
        memset(text + used, ' ', indentation);
        used += indentation;
        memcpy(text + used, binding, binding_length);
        used += binding_length;
    }
    assert(used + 1 == size);
    text[used] = 0;
    return text;
}

/** Deep finite applications use explicit frames; every real arena allocation
 * failure retains the caller's output and releases the entire partial owner.
 */
static void it_parses_deep_static_trees_and_unwinds_every_expression_allocation_failure(void) {
    char *text = nested_static_source(600);
    setup_tree();
    Parser parser = static_fixture(text);
    size_t before = allocation_count;
    AstTypeExpression *expression = NULL;
    assert(parser_type_expression(&parser, parser.line->code, 0, &expression));
    size_t allocations = allocation_count - before;
    for (size_t depth = 0; depth < 600; depth++) {
        assert(expression->first != NULL && expression->first == expression->last);
        expression = expression->first->input;
    }
    assert(expression != NULL && expression->first == NULL && parser.line == NULL);
    destroy_tree();
    assert(allocations != 0);
    for (size_t failure = 1; failure <= allocations; failure++) {
        setup_tree();
        parser = static_fixture(text);
        failure_at = allocation_count + failure;
        AstTypeExpression sentinel = {0};
        expression = &sentinel;
        assert(!parser_type_expression(&parser, parser.line->code, 0, &expression));
        assert(expression == &sentinel && module.error.diagnostic.code == DIAGNOSTIC_OUT_OF_MEMORY);
        destroy_tree();
    }
    free(text);
}
