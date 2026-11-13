/** @file
 * @brief Compare every explicit control successor and typed replacement/chain/loop operand across independent owners.
 */
/** All phases retain the same exact source/receiver, construction choice and commit boundaries. */
static void compare_replacement(const ReplacementPlan *left, const ReplacementPlan *right) {
    if (left == NULL || right == NULL) { assert(left == right); return; }
    assert(left != right && left->context == right->context && left->construction == right->construction);
    assert(left->statement->syntax == right->statement->syntax);
    assert((left->binding == NULL ? NULL : left->binding->syntax) == (right->binding == NULL ? NULL : right->binding->syntax));
    assert(left->intrinsic == right->intrinsic && left->position == right->position);
    assert(left->form == right->form && left->transfer == right->transfer && same_type(left->value_type, right->value_type));
    compare_symbol(left->field, right->field);
    compare_source(left->source, right->source);
    compare_access(left->receiver, right->receiver);
    compare_edge(left->first, right->first);
    compare_edge(left->commit, right->commit);
    compare_edge(left->last, right->last);
}
/** Every alternative/failure/resume and cross-type entry edge must preserve its unique selector and nominal owner. */
static void compare_controls(const PlannedType *left, const PlannedType *right) {
    compare_edge(left->entry, right->entry);
    compare_edge(left->complete, right->complete);
    compare_edge(left->yield, right->yield);
    compare_edge(left->failure, right->failure);
    compare_edge(left->resume_entry, right->resume_entry);
    const ControlNode *a = left->controls, *b = right->controls;
    for (; a != NULL && b != NULL; a = a->allocated_next, b = b->allocated_next) {
        assert(a != b && a->operation == b->operation && a->selector == b->selector && a->position == b->position);
        assert(same_span(a->site, b->site) && same_planned_type(a->type, b->type) && same_planned_type(a->callee, b->callee));
        assert(same_type(a->value_type, b->value_type) && a->value_form == b->value_form);
        assert(syntax_of(a->statement) == syntax_of(b->statement));
        assert((a->binding == NULL ? NULL : a->binding->syntax) == (b->binding == NULL ? NULL : b->binding->syntax));
        compare_symbol(a->field, b->field);
        compare_edge(a->next, b->next);
        compare_edge(a->alternative, b->alternative);
        compare_edge(a->failure, b->failure);
        compare_edge(a->resume, b->resume);
        assert(frame_slot(a->frame) == frame_slot(b->frame) && loop_syntax(a->loop) == loop_syntax(b->loop));
        assert((a->chain == NULL ? NULL : a->chain->root->syntax) == (b->chain == NULL ? NULL : b->chain->root->syntax));
        compare_replacement(a->replacement, b->replacement);
    }
    assert(a == NULL && b == NULL);
    const PlannedLoop *x = left->loops, *y = right->loops;
    for (; x != NULL && y != NULL; x = x->next, y = y->next) {
        assert(x != y && loop_syntax(x) == loop_syntax(y) && loop_syntax(x->parent) == loop_syntax(y->parent));
        assert(frame_slot(x->frame) == frame_slot(y->frame));
        compare_edge(x->entry, y->entry);
        compare_edge(x->repeat, y->repeat);
        compare_edge(x->test, y->test);
        compare_edge(x->body, y->body);
        compare_edge(x->exit, y->exit);
        compare_edge(x->after, y->after);
    }
    assert(x == NULL && y == NULL);
    const PlannedChain *c = left->chains, *d = right->chains;
    for (; c != NULL && d != NULL; c = c->next, d = d->next) {
        assert(c != d && c->root->syntax == d->root->syntax && frame_slot(c->root_frame) == frame_slot(d->root_frame));
        assert(loop_syntax(c->header) == loop_syntax(d->header));
        compare_edge(c->exit, d->exit);
        compare_edge(c->break_target, d->break_target);
    }
    assert(c == NULL && d == NULL);
    const ReplacementPlan *e = left->replacements, *f = right->replacements;
    for (; e != NULL && f != NULL; e = e->allocated_next, f = f->allocated_next) { compare_replacement(e, f); }
    assert(e == NULL && f == NULL);
}
