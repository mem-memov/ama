/** @file
 * @brief Verify exact loop roots, caller bodies and syntactically distinct traversal headers.
 */
/** One root owns the complete written path; inner loops and handlers retain separate contexts. */
static void it_folds_the_complete_condition_path_into_one_lexical_loop(void) {
    setup_tree();
    AstNode *outer = parsed("@!\na\n b !\n  < caller\n  condition[caller index]\n  # condition gap\n  {\n   c %\n    next condition\n    {\n     ^\n    }\n  }\n :\n  fallback !\n:\n >\nafter !\n");
    assert(outer->kind == AST_LOOP && outer->next->kind == AST_CALL);
    AstNode *command = outer->as.control.command;
    assert(command != outer && command->next == NULL && command->as.call.mode == AST_RETAINED_READ);
    AstNode *selected = command->as.call.first_output->operation;
    assert(selected->as.call.enclosing_operation == command && selected->as.call.mode == AST_ENTRY_CALL);
    assert(outer->as.control.condition_owner == selected && selected->as.call.input_count == 1);
    assert(outer->as.control.condition->index != NULL);
    assert(command->as.call.has_alternative && selected->as.call.has_alternative);
    AstNode *inner = outer->as.control.body.first;
    assert(inner->kind == AST_LOOP && inner->as.control.command->as.call.mode == AST_RESUME_CALL);
    assert(inner->as.control.body.first->kind == AST_BREAK);
    assert(outer->as.control.body_marker.start < outer->as.control.closing_marker.start);
    assert(module.warnings == NULL);
    destroy_tree();
    setup_tree();
    outer = parsed("@!\na\n condition\n {\n }\n");
    assert(outer->kind == AST_LOOP && outer->as.control.command != NULL);
    assert(outer->as.control.condition_owner == outer->as.control.command);
    assert(module.warnings != NULL && module.warnings->next == NULL);
    destroy_tree();
}
/** The reference binder is not an index and its collection may itself contain references. */
static void it_distinguishes_owned_indices_from_traversal_reference_binders(void) {
    const char *forms[] = {"@!\nlist[item&]\n{\n ^\n}\n", "@!\nlist&[current item&]\n{\n %\n}\n", "@!\nlist[index]\n{\n >\n}\n"};
    for (size_t index = 0; index < 3; index++) {
        setup_tree();
        AstNode *node = parsed(forms[index]);
        assert(node->kind == (index == 2 ? AST_LOOP : AST_FOREACH));
        if (index != 2) {
            assert(node->as.traversal.reference_elements == (index == 1));
            assert(node->as.traversal.binder.end > node->as.traversal.binder.start);
            assert(node->as.traversal.body.first != NULL);
            assert(node->as.traversal.closing_marker.source != NULL);
        } else { assert(node->as.control.condition->index != NULL); }
        destroy_tree();
    }
}
/** Header handlers cannot break the loop whose test was refused, but may break an enclosing loop body. */
static void it_uses_incoming_loop_context_for_header_handlers(void) {
    setup_tree();
    AstNode *node = parsed("@!\nouter\n{\n a !\n  condition\n  {\n   ^\n  }\n :\n  ^\n}\n");
    AstNode *inner = node->as.control.body.first;
    assert(inner->kind == AST_LOOP);
    assert(inner->as.control.command->as.call.alternative.first->kind == AST_BREAK);
    destroy_tree();
    setup_tree();
    ParseStatus status = parse_text("@!\na !\n condition\n {\n }\n:\n ^\n");
    assert(status == PARSE_INVALID && !module.valid);
    destroy_tree();
}
