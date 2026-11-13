/**
 * @file
 * @brief Verify exact colon depth and ordinary caller-context roots inside selected-operation handlers.
 */

/** Selected and root handlers retain distinct marker spans; calls in a handler are not selectors of the refused target. */
static void it_attaches_refusal_markers_to_their_exact_operations_and_resets_handler_selector_context(void) {
    setup_tree();
    AstNode *operation = root("@!\nroot !\n result !\n  > out\n # parent argument region\n :\n"
        "  # selected handler\n  local root !\n   nested !\n other !\n :\n  >\n:\n >\n");
    assert(operation->as.call.has_alternative && operation->as.call.alternative.first->kind == AST_RETURN);
    AstNode *selected = operation->as.call.first_output->operation;
    AstNode *other = operation->as.call.first_output->next->operation;
    assert(selected->as.call.has_alternative && other->as.call.has_alternative);
    AstNode *local = selected->as.call.alternative.first;
    assert(local->kind == AST_CALL && local->as.call.enclosing_operation == NULL);
    assert(layout_text_is(local->as.call.target->name, "local root"));
    assert(local->as.call.first_output->operation->as.call.enclosing_operation == local);
    SourcePosition position;
    bool located = source_position(operation->as.call.refusal_marker.source, operation->as.call.refusal_marker.start, &position);
    assert(located && position.column == 1);
    located = source_position(selected->as.call.refusal_marker.source, selected->as.call.refusal_marker.start, &position);
    assert(located && position.column == 2);
    assert(operation->span.end >= other->span.end && selected->span.end >= local->span.end);
    destroy_tree();
    setup_tree();
    operation = root("@!\nroot !\n child !\n :\n:\n");
    assert(operation->as.call.has_alternative && operation->as.call.alternative.first == NULL);
    selected = operation->as.call.first_output->operation;
    assert(selected->as.call.has_alternative && selected->as.call.alternative.first == NULL);
    destroy_tree();
}

/** Selection closes inputs even with zero ordinary outputs; handlers align with operations rather than bindings. */
static void it_rejects_input_after_selection_and_misindented_or_ineligible_handlers(void) {
    const char *invalid[] = {"@!\nroot !\n child !\n < source", "@!\nroot !\n child !\n  > receiver\n  :\n   >",
        "@!\n..palette\n method !", "@!\n..palette\n value >> receiver\n:\n >",
        "@!\nroot !\n methods&[] %", "@!\nroot !\n child\n", "@!\nroot !\n child !\n # child arguments closed\n  > receiver",
        "@!\nroot !\n child %\n  < source"};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(invalid[0]); index++) {
        setup_tree();
        ParseStatus status = parse_text(invalid[index]);
        assert(status == PARSE_INVALID && !module.valid && diagnostic_valid(&module.error.diagnostic));
        destroy_tree();
    }
}
