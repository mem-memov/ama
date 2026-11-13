/** @file
 * @brief Reject damaged lexical/chain identities and release every partial structured-lowering allocation.
 */
static const char structured_all[] = "@!\nitems[]\n.value\nproducer\n.producer\nsource\n.value\n"
    "producer\n left !\n  > first\n  next& %\n   > second\n  :\n   %\n"
    "items[item&]\n{\n items[]\n  length !\n   > count\n source\n :\n  ^\n %\n}\n"
    "items[]\n length !\n  value\n  {\n  }\n";
/** Fixed roots, body labels, chain/frame ownership and selected-action continuations cannot be guessed from other valid records. */
static void it_rejects_damaged_loop_frames_chain_roots_and_subtree_continuations(void) {
    for (size_t shape = 0; shape < 10; shape++) {
        prepare_structured(structured_all);
        PlannedLoop *traversal = plan.entry->loops;
        PlannedLoop *header = traversal->next;
        ControlNode *selected = find_control(plan.entry, CONTROL_SELECT_RESULT);
        ControlNode *action = NULL;
        for (ControlNode *node = plan.entry->controls; node != NULL; node = node->allocated_next) {
            if (node->operation == CONTROL_ACTION_NEXT && node->chain->header == NULL) { action = node; break; }
        }
        assert(action != NULL);
        switch (shape) {
            case 0: traversal->repeat = traversal->entry; break;
            case 1: header->repeat = header->test; break;
            case 2: header->test->next->next = header->entry; break;
            case 3: selected->frame = NULL; break;
            case 4: selected->chain = header->header; break;
            case 5: action->resume = plan.entry->complete; break;
            case 6: header->header->exit = header->entry; break;
            case 7: plan.entry->resume_entry = plan.entry->entry; break;
            case 8: selected->alternative = plan.entry->failure; break;
            case 9: traversal->repeat->loop = header; break;
        }
        size_t before = allocation_count;
        bool valid = program_plan_validate(&plan);
        assert(!valid && !plan.valid && plan.status == PLAN_INVALID && diagnostic_valid(&plan.error));
        assert(allocation_count == before);
        destroy_plan();
    }
}
/** Every queued operation/block, loop label, chain record and method staging prefix remains arena-owned after fail-at-N. */
static void it_unwinds_every_complete_structured_lowering_allocation(void) {
    prepare_structured(structured_all);
    program_plan_destroy(&plan);
    size_t live_before = live_allocations;
    program_plan_init(&plan, &semantic, sources.allocator);
    size_t before = allocation_count;
    PlanStatus status = program_plan_build(&plan);
    assert(status == PLAN_OK);
    size_t count = allocation_count - before;
    program_plan_destroy(&plan);
    assert(live_allocations == live_before);
    for (size_t failure = 1; failure <= count; failure++) {
        program_plan_init(&plan, &semantic, sources.allocator);
        failure_at = allocation_count + failure;
        status = program_plan_build(&plan);
        assert(status == PLAN_OUT_OF_MEMORY && !plan.valid && diagnostic_valid(&plan.error));
        program_plan_destroy(&plan);
        assert(live_allocations == live_before);
        failure_at = 0;
    }
    destroy_semantic();
}
