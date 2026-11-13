/**
 * @file
 * @brief Verify selector parent paths, ordered ordinary/selected actions and untouched caller binding metadata.
 */

/** The shorthand creates no local variable, ordinary parent binding or copy-source node. */
static void it_builds_the_short_direct_result_form_without_an_intermediate_receiver(void) {
    setup_tree();
    AstNode *operation = root("@!\nmod1\n mod2 !\n  > mod3\n");
    assert(operation->as.call.mode == AST_RETAINED_READ && operation->as.call.has_selected);
    assert(operation->as.call.first == NULL && operation->as.call.output_count == 0);
    AstOutputAction *action = operation->as.call.first_output;
    assert(action != NULL && action->kind == AST_SELECTED_ACTION && action->next == NULL);
    AstNode *selected = action->operation;
    assert(selected->as.call.enclosing_operation == operation && selected->as.call.mode == AST_ENTRY_CALL);
    assert(layout_text_is(selected->as.call.target->name, "mod2"));
    assert(selected->as.call.output_count == 1 && selected->as.call.first->position == 0);
    assert(layout_text_is(selected->as.call.first->receiver->name, "mod3"));
    assert(selected->as.call.first_output->binding == selected->as.call.first);
    destroy_tree();
}

/** Selectors have their own naming mode and consume no ordinary output position in the parent. */
static void it_keeps_sibling_action_order_and_binding_style_independent_of_selectors(void) {
    setup_tree();
    AstNode *operation = root("@!\nroot !\n input < caller value\n first > receiver\n method !\n"
        "  < caller value\n  > selected result\n second > receiver\n method& %\n  > resumed\n"
        " method\n  >> read value\n");
    assert(operation->as.call.named && operation->as.call.input_count == 1 && operation->as.call.output_count == 2);
    const AstOutputActionKind kinds[] = {AST_BINDING_ACTION,AST_SELECTED_ACTION,AST_BINDING_ACTION,AST_SELECTED_ACTION,AST_SELECTED_ACTION};
    AstOutputAction *action = operation->as.call.first_output;
    for (size_t index = 0; index < 5; index++) {
        assert(action != NULL && action->kind == kinds[index]);
        if (index == 0 || index == 2) { assert(action->binding->position == index / 2); }
        else {
            assert(action->operation->as.call.enclosing_operation == operation);
            assert(!action->operation->as.call.named);
        }
        action = action->next;
    }
    assert(action == NULL);
    AstNode *first = operation->as.call.first_output->next->operation;
    assert(first->as.call.mode == AST_ENTRY_CALL);
    assert(layout_text_is(first->as.call.first->source->access->name, "caller value"));
    AstNode *resumed = operation->as.call.first_output->next->next->next->operation;
    assert(resumed->as.call.mode == AST_RESUME_CALL && resumed->as.call.target->reference);
    destroy_tree();
}

/** An unmarked selected read is distinguished from a copy source by its required deeper action region. */
static void it_parses_unmarked_reads_and_whole_list_method_parents_as_operation_trees(void) {
    setup_tree();
    AstNode *operation = root("@!\nroot\n result value\n  >> receiver\n");
    AstNode *selected = operation->as.call.first_output->operation;
    assert(selected->as.call.mode == AST_RETAINED_READ);
    assert(layout_text_is(selected->as.call.target->name, "result value"));
    destroy_tree();
    setup_tree();
    operation = root("@!\nlist values[]\n append !\n  < item\n remove !\n  < position\n");
    assert(operation->as.call.target->whole_list && operation->as.call.output_count == 0);
    assert(operation->as.call.first_output->operation->as.call.mode == AST_ENTRY_CALL);
    assert(operation->as.call.first_output->next->operation->as.call.mode == AST_ENTRY_CALL);
    destroy_tree();
    setup_tree();
    operation = root("@!\nroot !\n methods&[positions[current row]] %\n  > caller values[positions[next row]]\n");
    selected = operation->as.call.first_output->operation;
    assert(selected->as.call.target->reference && selected->as.call.target->index != NULL);
    assert(layout_text_is(selected->as.call.target->index->index->name, "current row"));
    assert(layout_text_is(selected->as.call.first->receiver->index->index->name, "next row"));
    destroy_tree();
    setup_tree();
    ParseStatus status = parse_text("@!\nreceiver\n source value\n");
    assert(status == PARSE_OK && module.body.first->kind == AST_ASSIGNMENT);
    destroy_tree();
}
