/** @file
 * @brief Check complete loop restart roots and ordinary/intrinsic terminal values independently of nearest preceding calls.
 */
/** Header handlers stay in the outer loop; only actual loop/traversal bodies become new lexical break targets. */
static void it_retains_the_complete_restart_root_and_exact_handler_break_targets(void) {
    prepare_control();
    SemanticStatus status = resolve_control("@!\nouter\n.value\nroot\n.producer\nitems[]\n.value\ncondition\n.resource\n"
        "outer\n{\n root !\n  child !\n   condition\n   {\n    items[item&]\n    {\n     ^\n    }\n    ^\n   }\n  :\n   ^\n :\n  ^\n}\n");
    assert(status == SEMANTIC_OK);
    SemanticStatement *outer = control_statement(AST_LOOP, 0);
    SemanticStatement *inner = control_statement(AST_LOOP, 1);
    SemanticStatement *traversal = control_statement(AST_FOREACH, 0);
    assert(outer->loop_root == outer && outer->enclosing_loop == NULL);
    assert(inner->enclosing_loop == outer && inner->loop_root != inner->condition_owner);
    assert(inner->loop_root->syntax == inner->syntax->as.control.command);
    assert(inner->condition_owner->syntax == inner->syntax->as.control.condition_owner);
    assert(inner->loop_root->enclosing_loop == outer && inner->condition_owner->enclosing_loop == outer);
    assert(inner->condition_owner->enclosing_operation == inner->loop_root);
    assert(layout_text_is(inner->condition.field->name, "condition"));
    assert(inner->condition.field->owner != semantic_entry() && inner->condition.type == semantic_find_type(&semantic, ".value"));
    SemanticStatement *targets[] = {traversal, inner, outer, outer};
    for (size_t index = 0; index < 4; index++) { assert(control_statement(AST_BREAK, index)->enclosing_loop == targets[index]); }
    assert(traversal->enclosing_loop == inner && traversal->traversal->binding->local);
    destroy_semantic();
}
/** Mode tags remain explicit and selected indices remain caller-owned across all nine parent/child combinations. */
static void it_resolves_all_header_modes_and_indexed_terminal_results_in_caller_context(void) {
    prepare_control();
    file("generated/program/worker", "@>\nconditions[]\n.value\n@!\n");
    const char *modes[] = {" !", " %", ""};
    for (size_t root = 0; root < 3; root++) {
        for (size_t child = 0; child < 3; child++) {
            char text[512];
            int length = snprintf(text, sizeof(text), "@!\nroot\n.producer\nindex\n.value\n |0\n"
                "root%s\n child%s\n  conditions[index]\n  {\n  }\n", modes[root], modes[child]);
            assert(length > 0 && (size_t)length < sizeof(text));
            SemanticStatus status = resolve_control(text);
            assert(status == SEMANTIC_OK);
            SemanticStatement *loop = control_statement(AST_LOOP, 0);
            assert(loop->loop_root->syntax->as.call.mode == (root == 0 ? AST_ENTRY_CALL : root == 1 ? AST_RESUME_CALL : AST_RETAINED_READ));
            assert(loop->condition_owner->syntax->as.call.mode == (child == 0 ? AST_ENTRY_CALL : child == 1 ? AST_RESUME_CALL : AST_RETAINED_READ));
            assert(loop->condition.access->index->symbol->owner == semantic_entry() && !loop->condition.list && !loop->condition.reference);
            assert(loop->empty_body && loop->enclosing_loop == NULL);
            destroy_semantic();
        }
    }
}
/** The complete list-service root is restarted; length's terminal result is typed owned integer, not a caller-local field. */
static void it_types_intrinsic_terminal_conditions_without_fake_field_accesses(void) {
    prepare_control();
    SemanticStatus status = resolve_control("@!\nitems[]\n.value\nitems[]\n length !\n  value\n  {\n  }\n");
    assert(status == SEMANTIC_OK);
    SemanticStatement *loop = control_statement(AST_LOOP, 0);
    assert(loop->loop_root->operation_kind == SEMANTIC_LIST_SERVICE_OPERATION);
    assert(loop->condition_owner->operation_kind == SEMANTIC_LIST_METHOD_OPERATION);
    assert(loop->condition.type == semantic.list_integer && loop->condition.intrinsic && loop->condition.position == 0);
    assert(loop->condition.access == NULL && loop->condition.field == NULL && !loop->condition.reference);
    destroy_semantic();
    file("generated/standard/integer.abi", "+ initialize\n- destroy\n| literal\n= clone\n[] index\n");
    status = resolve_control("@!\nitems[]\n.value\nitems[]\n length !\n  value\n  {\n  }\n");
    assert(status == SEMANTIC_INVALID && strcmp(semantic.error.subject, "..integer") == 0);
    assert(layout_text_is(semantic.error.primary.span, "value") && diagnostic_valid(&semantic.error));
    destroy_semantic();
}
