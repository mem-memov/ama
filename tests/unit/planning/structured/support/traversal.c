/** @file
 * @brief Verify fixed cursor loops, scoped-reference reuse, and nonowning break/return/restart/resume cleanup.
 */
/** Live-boundary repetition uses distinct entry/next/leave labels; scope exits do not restore or destroy reused reference targets. */
static void it_lowers_foreach_scopes_and_resume_validation_with_fixed_cursor_labels(void) {
    prepare_structured("@!\nitems[]\n/test/fixture/v1.0.0/src/value\nsaved&\n/test/fixture/v1.0.0/src/value\nsource\n/test/fixture/v1.0.0/src/value\nitems[item&]\n{\n items[item&]\n {\n  source\n  :\n   ^\n }\n %\n >\n}\nitems[saved&]\n{\n}\n");
    PlannedLoop *outer = plan.entry->loops;
    PlannedLoop *inner = outer->next;
    PlannedLoop *empty = inner->next;
    assert(outer != NULL && inner != NULL && empty != NULL && empty->next == NULL);
    assert(inner->parent == outer && empty->parent == NULL);
    assert(plan.entry->entry->operation == CONTROL_RESET_SCOPES && plan.entry->entry->next == outer->entry);
    assert(outer->entry->operation == CONTROL_FOREACH_ENTER && outer->entry->next == outer->repeat);
    assert(outer->repeat->operation == CONTROL_FOREACH_NEXT && outer->repeat->alternative == outer->exit);
    assert(outer->exit->operation == CONTROL_FOREACH_LEAVE && outer->exit->next == empty->entry);
    assert(outer->body == inner->entry && inner->repeat->alternative == inner->exit);
    assert(outer->frame->cell == inner->frame->cell && outer->frame->origin == inner->frame->origin);
    assert(empty->frame->cell == NULL && empty->frame->persistent_binding == main_field("saved"));
    assert(empty->entry->operation == CONTROL_FOREACH_EMPTY && empty->entry->next == empty->after);
    assert(empty->test == NULL && empty->body == empty->after);
    ControlNode *condition = find_control(plan.entry, CONTROL_CONDITION);
    assert(condition != NULL && condition->statement->negative_branch);
    assert(condition->next == inner->repeat && condition->alternative == inner->exit);
    ControlNode *yield = find_control(plan.entry, CONTROL_SAVE_POSITION);
    assert(yield != NULL && inner->after == yield && yield->next == plan.entry->yield);
    assert(yield->resume->operation == CONTROL_CLEANUP_SCOPES && yield->resume->loop == outer);
    assert(yield->resume->next == plan.entry->complete);
    assert(plan.entry->resume_entry->operation == CONTROL_VALIDATE_TRAVERSALS);
    assert(plan.entry->resume_entry->next->operation == CONTROL_RESUME_SAVED);
    assert(plan.entry->resume_entry->next->next == NULL);
    destroy_plan();
}
