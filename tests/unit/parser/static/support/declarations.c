/** @file
 * @brief Verify complete declaration trees, selected nested values and initializer ownership.
 */

/** Child selectors close their exact input expression before parent siblings and runtime initializers. */
static void it_preserves_nested_declaration_inputs_selectors_and_initializer_spans(void) {
    setup_tree();
    const char *text = "@<\nargument\n./outer\n .first < .input type\n .second < ~factory\n"
        "  .slot < ./inner\n   .element < .other type\n  .result\n .last < ./leaf\n"
        "@>\noutput&[]\n./outer\n .first < .input type\n@!\nlocal\n~factory\n"
        " .slot < ./inner\n  .element < //integer\n .result\n |payload\nsibling\n./leaf\n";
    assert(parse_text(text) == PARSE_OK && module.valid);
    const AstTypeExpression *outer = module.parameters.first->as.declaration.type;
    assert(layout_text_is(outer->head, "./outer") && outer->selected_export.source == NULL);
    const AstStaticBinding *first = outer->first, *second = first->next, *last = second->next;
    assert(last == outer->last && last->next == NULL);
    assert(layout_text_is(first->label, ".first") && layout_text_is(first->input->head, ".input type"));
    const AstTypeExpression *factory = second->input;
    assert(layout_text_is(factory->head, "~factory") && layout_text_is(factory->selected_export, ".result"));
    assert(second->span.end == factory->selected_export.end);
    const AstTypeExpression *inner = factory->first->input;
    assert(layout_text_is(inner->head, "./inner") && layout_text_is(inner->first->input->head, ".other type"));
    assert(outer->span.end == last->input->span.end && second->span.end < last->span.start);
    const AstNode *local = module.body.first;
    assert(local->kind == AST_DECLARATION && local->next && local->next->kind == AST_DECLARATION);
    assert(local->as.declaration.initializer_count == 1);
    assert(layout_text_is(local->as.declaration.type->selected_export, ".result"));
    const AstInitializer *initializer = local->as.declaration.first;
    assert(local->as.declaration.type->span.end < initializer->source->span.start);
    assert(layout_text_is(initializer->source->payload, "payload"));
    assert(local->span.end == initializer->source->span.end && local->span.end < local->next->span.start);
    assert(module.results.first->as.declaration.name->reference && module.results.first->as.declaration.name->whole_list);
    destroy_tree();
}

/** Source parsing neither resolves the head nor uses callee metadata to reinterpret a bare selector. */
static void it_parses_declaration_arguments_before_lookup_and_in_each_ordinary_section(void) {
    const char *sources[] = {
        "@=\nshared\n./unknown\n .input < //integer\n .out\n |0\n@!\n",
        "@<\nargument\n./unknown\n .input < .type\n .out\n@C\n",
        "@>\nresult\n./unknown\n .input < .type\n .out\n@!\n",
        "@!\nvalue\n//integer\n .unknown output\n"
    };
    for (size_t index = 0; index < sizeof(sources) / sizeof(*sources); index++) {
        setup_tree();
        assert(parse_text(sources[index]) == PARSE_OK && module.valid);
        destroy_tree();
    }
}

/** A completed selector or initializer cannot reopen its expression, and graph
 * values/namespaces do not acquire the declaration-only selection/input form.
 */
static void it_rejects_crossed_expression_phases_outputs_and_nested_runtime_initializers(void) {
    const char *invalid[] = {
        "@!\nvalue\n./factory\n .out\n .input < //integer\n",
        "@!\nvalue\n./factory\n .input < ./inner\n  .out\n  .later < //integer\n",
        "@!\nvalue\n./factory\n .out > .alias\n",
        "@!\nvalue\n./outer\n .input < ./inner\n  |0\n",
        "@!\nvalue\n./outer\n .input < ./inner\n  .out\n   .child < //integer\n",
        "@!\nvalue\n./outer\n |0\n .input < //integer\n",
        "@!\nvalue\n./outer\n . < //integer\n",
        "@!\nvalue\n./outer\n .input <\n",
        "@...\n.type\n./outer\n .input < ./inner\n  .out\n",
        "@!\n./factory\n .input < //integer\n zero >> value\n"
    };
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        setup_tree();
        assert(parse_text(invalid[index]) == PARSE_INVALID && !module.valid);
        assert(diagnostic_valid(&module.error.diagnostic));
        destroy_tree();
    }
}
