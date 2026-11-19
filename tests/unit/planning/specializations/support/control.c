/** @file
 * @brief Check exact concrete call targets, parameter/result types and retained traversal/yield control.
 * @see [T187 contract](../../../../../docs/implementation/specialized-planning.md)
 */

/** Calls retain concrete callee symbols and body entries even though both variants share original AST sites. */
static void it_links_each_call_to_its_exact_concrete_body_and_interface(void) {
    setup_specializations();
    ProgramPlan *plan = &frontend.plan;
    size_t entered = 0, installed = 0, initialized = 0, transferred = 0, connected = 0;
    for (const ControlNode *node = plan->entry->controls; node; node = node->allocated_next) {
        if (node->operation == CONTROL_ENTER_INSTANCE) {
            assert(node->statement && node->callee->type == node->statement->namespace_type);
            assert(node->next == node->callee->entry && node->selector);
            entered++;
        }
        if (node->operation == CONTROL_SET_RETURN) {
            assert(node->resume && node->next && node->next->operation == CONTROL_ENTER_INSTANCE);
            assert(node->next->callee == node->callee && node->callee->type == node->statement->namespace_type);
            connected++;
        }
        if (node->operation == CONTROL_INSTALL_PARAMETER || node->operation == CONTROL_INITIALIZE_RESULT) {
            assert(node->field && node->field->owner == node->statement->namespace_type);
            assert(node->field->type == node->value_type && node->field->role_ordinal == 0);
            assert(node->site.source == plan->entry->type->module->source->text);
            installed += node->operation == CONTROL_INSTALL_PARAMETER;
            initialized += node->operation == CONTROL_INITIALIZE_RESULT;
        }
        transferred += node->operation == CONTROL_TRANSFER_OUTPUT;
    }
    assert(entered == 2 && connected == 2 && installed == 2 && initialized == 2 && transferred == 2);
    const char *names[] = {"first", "second"};
    for (size_t binding = 0; binding < 2; binding++) {
        const PlannedType *worker = fixture_field(plan->entry, names[binding])->type;
        size_t yields = 0, traversals = 0, positions = 0;
        for (const ControlNode *node = worker->controls; node; node = node->allocated_next) {
            assert(node->type == worker);
            yields += node->operation == CONTROL_YIELD;
            if (node->operation == CONTROL_SAVE_POSITION) {
                assert(node->next == worker->yield && node->resume && node->statement);
                positions++;
            }
            traversals += node->operation == CONTROL_FOREACH_ENTER;
        }
        assert(yields == 1 && positions == 2 && traversals == 1 && worker->resume_entry && worker->cells && worker->frames);
    }
    size_t before = allocation_count;
    assert(program_plan_validate(plan) && plan->valid && allocation_count == before);
    destroy_frontend();
}
