/** @file
 * @brief Compare logical layouts and fixed lexical storage across independently owned complete plans.
 */
/** Canonical module identity is borrowed from one immutable Project; planned records themselves must remain independent. */
static bool same_planned_type(const PlannedType *left, const PlannedType *right) {
    return left == NULL || right == NULL ? left == right : same_type(left->type, right->type);
}
/** Control identity includes its owner and unique checked selector, with null distinct from every executable label. */
static void compare_edge(const ControlNode *left, const ControlNode *right) {
    if (left == NULL || right == NULL) { assert(left == right); return; }
    assert(left != right && left->selector == right->selector && same_planned_type(left->type, right->type));
}
/** Original unique statement syntax identifies a lexical loop, independently of plan allocation addresses. */
static const AstNode *loop_syntax(const PlannedLoop *loop) { return loop == NULL ? NULL : loop->statement->syntax; }
/** A null frame is distinct from source slot zero; each comparison already belongs to the same nominal owner. */
static size_t frame_slot(const StorageFrame *frame) { return frame == NULL ? 0 : frame->slot + 1; }
/** Compare stable field roles and all fixed frame/cell/origin identities, including shared versus reused storage. */
static void compare_storage(const PlannedType *left, const PlannedType *right) {
    assert(left != right && same_type(left->type, right->type));
    assert(left->field_count == right->field_count && left->layout_ordinal == right->layout_ordinal);
    assert(left->service_storage == right->service_storage && left->requires_pool == right->requires_pool);
    assert(left->stable_slots == right->stable_slots && left->frame_count == right->frame_count);
    assert(left->cell_count == right->cell_count && left->origin_count == right->origin_count);
    const StorageField *a = left->first, *b = right->first;
    for (; a != NULL && b != NULL; a = a->next, b = b->next) {
        assert(a != b && a->slot == b->slot && a->form == b->form && same_planned_type(a->type, b->type));
        compare_symbol(a->symbol, b->symbol);
        assert((a->previous == NULL ? 0 : a->previous->slot + 1) == (b->previous == NULL ? 0 : b->previous->slot + 1));
    }
    assert(a == NULL && b == NULL);
    const StorageFrame *x = left->frames, *y = right->frames;
    for (; x != NULL && y != NULL; x = x->next, y = y->next) {
        assert(x != y && x->statement->syntax == y->statement->syntax && x->slot == y->slot);
        assert(x->kind == y->kind && x->contents == y->contents);
        assert(frame_slot(x->operation_parent) == frame_slot(y->operation_parent));
        assert(frame_slot(x->traversal_parent) == frame_slot(y->traversal_parent));
        compare_symbol(x->persistent_binding, y->persistent_binding);
        compare_symbol(x->cell == NULL ? NULL : x->cell->symbol, y->cell == NULL ? NULL : y->cell->symbol);
        compare_symbol(x->origin == NULL ? NULL : x->origin->symbol, y->origin == NULL ? NULL : y->origin->symbol);
    }
    assert(x == NULL && y == NULL);
    const ScopedStorageCell *c = left->cells, *d = right->cells;
    for (; c != NULL && d != NULL; c = c->next, d = d->next) {
        assert(c != d && c->slot == d->slot && c->introduction->syntax == d->introduction->syntax);
        compare_symbol(c->symbol, d->symbol);
    }
    assert(c == NULL && d == NULL);
    const ReferenceOriginStorage *e = left->origins, *f = right->origins;
    for (; e != NULL && f != NULL; e = e->next, f = f->next) {
        assert(e != f && e->slot == f->slot);
        compare_symbol(e->symbol, f->symbol);
    }
    assert(e == NULL && f == NULL);
}
/** Lifecycle allocation rank identifies every normal/failure endpoint without comparing independent arena addresses. */
static size_t life_slot(const PlannedType *type, const LifeNode *target) {
    if (target == NULL) { return 0; }
    size_t slot = 1;
    for (const LifeNode *node = type->lifecycles; node != NULL; node = node->allocated_next, slot++) {
        if (node == target) { return slot; }
    }
    assert(false);
    return 0;
}
/** Life instructions compare by exact recipe order and field/ABI source identities, not native callback spellings alone. */
static void compare_lifecycle(const PlannedType *owner_left, const PlannedType *owner_right,
    const LifeProcedure *left, const LifeProcedure *right) {
    assert(life_slot(owner_left, left->first) == life_slot(owner_right, right->first));
    assert(life_slot(owner_left, left->last) == life_slot(owner_right, right->last));
    assert(life_slot(owner_left, left->failure) == life_slot(owner_right, right->failure));
    assert(left->accumulates_errors == right->accumulates_errors);
    const LifeNode *a = left->first, *b = right->first;
    for (; a != NULL && b != NULL; a = a->next, b = b->next) {
        assert(a != b && a->operation == b->operation && same_planned_type(a->type, b->type));
        assert(same_span(a->site, b->site));
        compare_symbol(a->field == NULL ? NULL : a->field->symbol, b->field == NULL ? NULL : b->field->symbol);
        if (a->binding == NULL || b->binding == NULL) { assert(a->binding == b->binding); }
        else { assert(a->binding->present == b->binding->present && same_span(a->binding->function, b->binding->function)); }
        assert(life_slot(owner_left, a->failure) == life_slot(owner_right, b->failure));
        assert(life_slot(owner_left, a->next) == life_slot(owner_right, b->next));
    }
    assert(a == NULL && b == NULL);
    a = left->failure;
    b = right->failure;
    for (; a != NULL && b != NULL; a = a->next, b = b->next) {
        assert(a != b && a->operation == b->operation && same_span(a->site, b->site));
    }
    assert(a == NULL && b == NULL);
}
