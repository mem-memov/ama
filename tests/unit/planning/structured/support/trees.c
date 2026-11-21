/** @file
 * @brief Check actual-result selection, depth-first sibling continuation and pre-handler whole-chain unwinding.
 */
/** No selected name creates an intermediate variable; caller input/output operands and returned reference targets retain their exact environments. */
static void it_keeps_selected_targets_and_caller_operands_through_depth_first_actions(void) {
    prepare_structured_types();
    PlanStatus status = plan_text("@!\nproducer\n/test/fixture/v1.0.0/src/producer\nsource\n/test/fixture/v1.0.0/src/value\nsink\n/test/fixture/v1.0.0/src/value\nproducer !\n < source\n left !\n  < source\n  > sink\n  next& %\n   > sink\n  :\n   %\n right& !\n  > sink\nproducer !\n");
    assert(status == PLAN_OK);
    assert(main_field("left") == NULL && main_field("next") == NULL && main_field("right") == NULL);
    PlannedChain *chain = NULL;
    for (PlannedChain *value = plan.entry->chains; value != NULL; value = value->next) {
        if (value->root->syntax->as.call.has_selected) { chain = value; break; }
    }
    assert(chain != NULL && chain->root_frame != NULL && chain->header == NULL);
    const ControlNode *retire = NULL;
    for (const ControlNode *node = plan.entry->controls; node != NULL; node = node->allocated_next) {
        if (node->operation == CONTROL_RETIRE_RETURN && node->statement == chain->root) { retire = node; break; }
    }
    assert(retire != NULL && retire->next->operation == CONTROL_ACTION_NEXT);
    const ControlNode *left_action = retire->next;
    const ControlNode *left_select = left_action->next;
    assert(left_select->operation == CONTROL_SELECT_RESULT);
    assert(left_select->frame->operation_parent == chain->root_frame);
    assert(left_select->statement->access->symbol->owner == chain->root->namespace_type);
    assert(left_select->statement->access->symbol->role == SYMBOL_RESULT);
    const SemanticBinding *input = left_select->statement->bindings;
    assert(input->source->access->symbol == main_field("source"));
    assert(input->next->receiver->symbol == main_field("sink"));
    const ControlNode *right_action = left_action->resume;
    assert(right_action->operation == CONTROL_ACTION_NEXT && right_action->next->operation == CONTROL_SELECT_RESULT);
    assert(right_action->statement->target_reference && right_action->statement->access->reference);
    const ControlNode *nested = NULL;
    for (const ControlNode *node = plan.entry->controls; node != NULL; node = node->allocated_next) {
        if (node->operation == CONTROL_SELECT_RESULT && node->statement->enclosing_operation == left_action->statement) { nested = node; break; }
    }
    assert(nested != NULL && nested->statement->syntax->as.call.mode == AST_RESUME_CALL);
    assert(nested->frame->operation_parent == left_action->frame);
    assert(nested->alternative->operation == CONTROL_ABANDON_CHAIN && nested->alternative->chain == chain);
    assert(nested->alternative->next->operation == CONTROL_REFUSAL_SNAPSHOT);
    assert(nested->alternative->next->next->operation == CONTROL_SAVE_POSITION);
    assert(nested->alternative->next->next->chain == NULL);
    assert(nested->alternative->next->next->resume == chain->exit);
    assert(chain->exit->operation == CONTROL_ACQUIRE_INSTANCE);
    assert(left_action->resume != chain->exit && right_action->resume->operation == CONTROL_RELEASE_TARGET);
    destroy_plan();
}
/** Intrinsic methods remain caller frames with typed preflight/prepare/commit and can immediately execute their real returned element. */
static void it_lowers_list_methods_and_ephemeral_outputs_without_synthetic_module_bodies(void) {
    prepare_structured("@!\nitems[]\n/test/fixture/v1.0.0/src/value\nsource\n/test/fixture/v1.0.0/src/value\nitems[]\n append !\n  < & source\n  > result&\n  element& !\n length !\n  > count\n");
    assert(plan.entry->frame_count == 4 && plan.context_storage.pools == 3);
    ControlNode *service = find_control(plan.entry, CONTROL_ACQUIRE_LIST);
    assert(service != NULL && service->chain->root_frame == service->frame);
    assert(service->next->operation == CONTROL_ACTION_NEXT);
    const ControlNode *stage = service->next->next;
    assert(stage->operation == CONTROL_EVALUATE_INPUT);
    stage = stage->next;
    assert(stage->operation == CONTROL_STAGE_METHOD_INPUT && stage->field == NULL);
    assert(stage->replacement->context == REPLACE_METHOD_PARAMETER && stage->replacement->intrinsic != NULL);
    assert(stage->replacement->form == STORAGE_REFERENCE && stage->replacement->transfer == TRANSFER_HANDLE);
    const ControlNode *preflight = stage->replacement->last->next;
    assert(preflight->operation == CONTROL_LIST_METHOD_PREFLIGHT);
    assert(preflight->alternative->operation == CONTROL_ABANDON_CHAIN);
    assert(preflight->next->operation == CONTROL_LIST_METHOD_PREPARE);
    assert(preflight->next->next->operation == CONTROL_LIST_METHOD_COMMIT);
    const ControlNode *output = preflight->next->next->next;
    assert(output->operation == CONTROL_TRANSFER_OUTPUT && output->binding->field == NULL);
    const ControlNode *element = output->replacement->last->next;
    assert(element->operation == CONTROL_ACTION_NEXT && element->next->operation == CONTROL_SELECT_RESULT);
    assert(element->statement->intrinsic_target && element->statement->target_reference);
    assert(element->statement->namespace_type == semantic_find_type(&semantic, "/test/fixture/v1.0.0/src/value"));
    assert(structured_operation(CONTROL_LIST_METHOD_COMMIT, 0)->statement->operation_kind == SEMANTIC_LIST_METHOD_OPERATION);
    destroy_plan();
}
