/** @file
 * @brief Verify exact full-header repeats, one-check empty loops and incoming handler break targets.
 */
/** Every test starts at its own lexical root; conversion precedes complete header release and no grant enters the body. */
static void it_repeats_complete_headers_and_releases_protection_before_body_or_exit(void) {
    const char *bodies[] = {"", "   %\n", "   local\n   /test/fixture/v1.0.0/src/value\n"};
    for (size_t shape = 0; shape < sizeof(bodies) / sizeof(*bodies); shape++) {
        char text[1024];
        int size = snprintf(text, sizeof(text), "@!\nproducer\n/test/fixture/v1.0.0/src/producer\nsource\n/test/fixture/v1.0.0/src/value\nproducer !\n < source\n left %%\n  condition\n  {\n%s  }\n", bodies[shape]);
        assert(size > 0 && (size_t)size < sizeof(text));
        prepare_structured(text);
        PlannedLoop *loop = plan.entry->loops;
        assert(loop != NULL && loop->next == NULL && loop->parent == NULL);
        assert(loop->entry == loop->repeat && loop->entry->operation == CONTROL_HEADER_ENTER);
        assert(loop->entry->next->operation == CONTROL_ACQUIRE_INSTANCE);
        assert(loop->entry->next->statement == loop->statement->loop_root);
        assert(loop->test->operation == CONTROL_CONDITION && loop->test->frame != NULL);
        assert(loop->test->frame->statement == loop->statement->condition_owner);
        assert(loop->test->next->operation == CONTROL_RELEASE_CHAIN);
        assert(loop->test->alternative->operation == CONTROL_RELEASE_CHAIN);
        assert(loop->test->next->chain == loop->header && loop->test->alternative->chain == loop->header);
        assert(loop->test->next->next == loop->body && loop->test->alternative->next == loop->after);
        if (shape == 0) {
            assert(loop->statement->empty_body && loop->body == loop->after);
        } else if (shape == 1) {
            assert(!loop->statement->empty_body && loop->body->operation == CONTROL_SAVE_POSITION);
            assert(loop->body->resume == loop->repeat && loop->body->next == plan.entry->yield);
        } else { assert(!loop->statement->empty_body && loop->body == loop->repeat); }
        destroy_plan();
    }
}
/** A refused header link unwinds before its handler; ^ in that handler exits the incoming outer loop, not the refused loop. */
static void it_keeps_header_handlers_in_the_incoming_loop_context_after_chain_abandonment(void) {
    prepare_structured("@!\nouter\n/test/fixture/v1.0.0/src/value\nproducer\n/test/fixture/v1.0.0/src/producer\nouter\n{\n producer !\n  left %\n   condition\n   {\n    ^\n   }\n  :\n   ^\n}\n");
    PlannedLoop *outer = plan.entry->loops;
    PlannedLoop *inner = outer->next;
    assert(outer != NULL && inner != NULL && inner->parent == outer && inner->header != NULL);
    assert(inner->header->break_target == outer->exit && inner->header->exit == inner->after);
    assert(inner->body == inner->exit && inner->after == outer->repeat);
    ControlNode *selected = find_control(plan.entry, CONTROL_SELECT_RESULT);
    assert(selected != NULL && selected->chain == inner->header);
    assert(selected->alternative->operation == CONTROL_ABANDON_CHAIN);
    assert(selected->alternative->next->operation == CONTROL_REFUSAL_SNAPSHOT);
    assert(selected->alternative->next->next == outer->exit);
    assert(outer->exit != inner->exit);
    destroy_plan();
}
/** A header can convert an intrinsic owned result directly while its method/service frames remain protected. */
static void it_converts_intrinsic_header_results_without_an_intermediate_receiver(void) {
    prepare_structured("@!\nitems[]\n/test/fixture/v1.0.0/src/value\nitems[]\n length !\n  value\n  {\n  }\n");
    PlannedLoop *loop = plan.entry->loops;
    assert(loop != NULL && loop->statement->condition.intrinsic && loop->statement->condition.field == NULL);
    assert(loop->statement->condition.type == semantic.list_integer && loop->test->frame != NULL);
    assert(loop->test->frame->statement->operation_kind == SEMANTIC_LIST_METHOD_OPERATION);
    assert(loop->test->frame->operation_parent == loop->header->root_frame);
    ControlNode *commit = find_control(plan.entry, CONTROL_LIST_METHOD_COMMIT);
    assert(commit != NULL && commit->next == loop->test);
    assert(find_control(plan.entry, CONTROL_TRANSFER_OUTPUT) == NULL);
    assert(main_field("value") == NULL && loop->body == loop->after);
    destroy_plan();
}
