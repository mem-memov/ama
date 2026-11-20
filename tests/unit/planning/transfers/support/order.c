/** @file
 * @brief Check CALL-08's distinct input selection, declaration installation and output text order.
 */
/** All supplied selections precede aggregate conflict preflight; each parameter/result then commits independently with one construction path. */
static void it_lowers_source_protection_and_individual_commits_in_the_three_required_orders(void) {
    prepare_transfers();
    PlanStatus status = plan_text(transfer_main);
    assert(status == PLAN_OK);
    ControlNode *acquire = plan.entry->entry;
    assert(acquire->operation == CONTROL_ACQUIRE_INSTANCE);
    assert(acquire->alternative->operation == CONTROL_REFUSAL_SNAPSHOT);
    assert(acquire->failure->operation == CONTROL_CAPTURE_ERROR);
    ControlNode *node = acquire->next;
    assert(node->operation == CONTROL_EVALUATE_INPUT && node->binding->position == 1);
    const SemanticSource *selected = node->binding->source;
    node = node->next;
    assert(node->operation == CONTROL_PREFLIGHT_INPUTS && node->failure == acquire->failure);
    node = node->next;
    for (size_t parameter = 0; parameter < 4; parameter++) {
        assert(node->operation == CONTROL_INSTALL_PARAMETER && node->position == parameter);
        const ReplacementPlan *recipe = node->replacement;
        if (parameter == 0) {
            assert(recipe->construction == REPLACEMENT_SUPPLIED_LITERAL && recipe->transfer == TRANSFER_LITERAL);
            assert(recipe->source->kind == AST_LITERAL_SOURCE);
            assert(layout_text_is(recipe->source->syntax->payload, "literal % ref& # bytes  "));
        } else if (parameter == 1) {
            assert(recipe->construction == REPLACEMENT_SUPPLIED_SOURCE && recipe->source == selected);
            assert(recipe->transfer == TRANSFER_CLONE && recipe->receiver == NULL);
        } else {
            assert(recipe->construction == REPLACEMENT_DECLARED_DEFAULT && recipe->source == NULL);
            assert(recipe->transfer == TRANSFER_DEFAULT);
            if (parameter == 2) {
                assert(recipe->field->initializers->source->access->symbol->role == SYMBOL_PARAMETER);
                assert(recipe->field->initializers->source->access->symbol->role_ordinal == 0);
            } else { assert(!recipe->field->type->cloneable && recipe->form == STORAGE_REFERENCE); }
        }
        node = after_replacement(node, REPLACE_PARAMETER);
    }
    for (size_t result = 0; result < 2; result++) {
        assert(node->operation == CONTROL_INITIALIZE_RESULT && node->position == result);
        assert(node->replacement->construction == REPLACEMENT_DECLARED_DEFAULT);
        node = after_replacement(node, REPLACE_RESULT);
    }
    assert(node->operation == CONTROL_SET_RETURN && node->next->operation == CONTROL_ENTER_INSTANCE);
    assert(node->next->next == node->next->callee->entry);
    assert(node->resume->operation == CONTROL_RETIRE_RETURN);
    ControlNode *output = node->resume->next;
    const SemanticAccess *receiver = output->binding->receiver;
    for (size_t index = 0; index < 2; index++) {
        assert(output->operation == CONTROL_TRANSFER_OUTPUT && output->binding->position == 1 - index);
        const ReplacementPlan *recipe = output->replacement;
        assert(recipe->construction == REPLACEMENT_RETAINED_RESULT && recipe->source == output->binding->source);
        assert(recipe->field == output->binding->field && recipe->receiver == output->binding->receiver);
        assert(recipe->receiver->symbol == receiver->symbol);
        output = after_replacement(output, REPLACE_OUTPUT);
    }
    assert(output->operation == CONTROL_RELEASE_TARGET && output->next == plan.entry->complete);
    assert(acquire->alternative->next == plan.entry->complete);
    destroy_plan();
}
/** Invocation modes exclude reset/body work exactly where the specification requires; constant reads also exclude access grants. */
static void it_omits_entry_preparation_for_resume_retained_and_constant_reads(void) {
    const char *operations[] = {"worker !\n > sink\n", "worker %\n > sink\n", "worker\n >> sink\n", "/test/fixture/v1.0.0/src/palette\n >> sink\n"};
    for (size_t mode = 0; mode < 4; mode++) {
        prepare_types();
        file("generated/program/packages/test/fixture/v1.0.0/src/worker", "@<\ninput\n/test/fixture/v1.0.0/src/value\n@>\noutput\n/test/fixture/v1.0.0/src/value\n@!\n");
        file("generated/program/packages/test/fixture/v1.0.0/src/palette", "@=\nvalue\n/test/fixture/v1.0.0/src/value\n |1\n");
        char text[256];
        int length = snprintf(text, sizeof(text), "@!\nworker\n/test/fixture/v1.0.0/src/worker\nsink\n/test/fixture/v1.0.0/src/value\n%s", operations[mode]);
        assert(length > 0 && (size_t)length < sizeof(text));
        PlanStatus status = transfer_plan_text(text);
        assert(status == PLAN_OK);
        assert((find_control(plan.entry, CONTROL_PREFLIGHT_INPUTS) != NULL) == (mode == 0));
        assert((find_control(plan.entry, CONTROL_INSTALL_PARAMETER) != NULL) == (mode == 0));
        assert((find_control(plan.entry, CONTROL_SET_RETURN) != NULL) == (mode < 2));
        assert((find_control(plan.entry, CONTROL_ACQUIRE_INSTANCE) != NULL) == (mode < 3));
        ControlNode *output = find_control(plan.entry, CONTROL_TRANSFER_OUTPUT);
        assert(output != NULL);
        ControlNode *after = after_replacement(output, REPLACE_OUTPUT);
        assert(after->operation == (mode == 3 ? CONTROL_COMPLETE : CONTROL_RELEASE_TARGET));
        destroy_plan();
    }
}
/** Transfer modes retain receiver storage form and original source identity; reference outputs copy handles into declared reference cells. */
static void it_preserves_assignment_and_reference_output_modes_without_snapshot_sources(void) {
    prepare_types();
    file("generated/program/packages/test/fixture/v1.0.0/src/worker", "@>\noutput&\n/test/fixture/v1.0.0/src/value\n@!\n");
    PlanStatus status = transfer_plan_text("@!\nsource\n/test/fixture/v1.0.0/src/value\ntarget\n/test/fixture/v1.0.0/src/value\nreference&\n/test/fixture/v1.0.0/src/value\nworker\n/test/fixture/v1.0.0/src/worker\n"
        "reference&\n &source\ntarget\n source\ntarget\n % source\nworker\n output& >> reference&\n");
    assert(status == PLAN_OK);
    const SemanticTransfer expected[] = {TRANSFER_HANDLE, TRANSFER_CLONE, TRANSFER_CONTINUATION};
    ControlNode *node = plan.entry->entry;
    for (size_t index = 0; index < sizeof(expected) / sizeof(*expected); index++) {
        assert(node->operation == CONTROL_SELECT_RECEIVER);
        const ReplacementPlan *recipe = node->replacement;
        assert(recipe->transfer == expected[index] && recipe->source == node->statement->source);
        assert(recipe->receiver == node->statement->access);
        assert(recipe->construction == (expected[index] == TRANSFER_LITERAL
            ? REPLACEMENT_SUPPLIED_LITERAL : REPLACEMENT_SUPPLIED_SOURCE));
        node = after_replacement(node, REPLACE_ASSIGNMENT);
    }
    assert(node->operation == CONTROL_ACQUIRE_INSTANCE);
    ControlNode *output = node->next;
    assert(output->operation == CONTROL_TRANSFER_OUTPUT);
    assert(output->binding->reference && output->value_form == STORAGE_REFERENCE);
    assert(output->replacement->form == STORAGE_REFERENCE && output->replacement->transfer == TRANSFER_HANDLE);
    assert(output->replacement->source == output->binding->source && output->replacement->source->reference);
    assert(after_replacement(output, REPLACE_OUTPUT)->operation == CONTROL_RELEASE_TARGET);
    destroy_plan();
}
