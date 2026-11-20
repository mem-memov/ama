/** @file
 * @brief Check the general representation, exact layout ordering and fixed lexical state sharing.
 */
/** Context values, permanent slots, interface state, cursors and conditional cells have distinct owners and lifetimes. */
static void it_separates_context_permanent_interface_and_fixed_lexical_storage(void) {
    prepare_storage(storage_main);
    bool built = plan_storage(&plan);
    assert(built && plan.storage_ready && !plan.valid);
    assert(plan.context_storage.pools == 4 && plan.context_storage.constants == 1);
    assert(plan.entry->stable_slots && plan.entry->field_count == 5);
    assert(plan.entry->frame_count == 6 && plan.entry->cell_count == 2 && plan.entry->origin_count == 3);
    assert((plan.entry->service_storage & STORAGE_CONSTRUCTING_IDENTITY) != 0);
    assert((plan.entry->service_storage & STORAGE_OWNERSHIP) != 0);
    StorageFrame *outer = frame_at(0);
    StorageFrame *producer = frame_at(1);
    StorageFrame *child = frame_at(2);
    StorageFrame *nested = frame_at(3);
    StorageFrame *sibling = frame_at(4);
    StorageFrame *reused = frame_at(5);
    assert(outer->kind == STORAGE_TRAVERSAL_FRAME && outer->cell != NULL);
    assert(outer->cell == nested->cell && outer->origin == nested->origin);
    assert(sibling->cell != outer->cell && sibling->origin != outer->origin);
    assert(reused->cell == NULL && reused->persistent_binding != NULL && reused->origin != NULL);
    assert(producer->operation_parent == NULL && child->operation_parent == producer);
    assert(producer->traversal_parent == outer && child->traversal_parent == outer);
    assert(nested->traversal_parent == outer && sibling->traversal_parent == NULL);
    assert((outer->contents & FRAME_BODY_RETURN) == 0 && (outer->contents & FRAME_BOUNDARY) != 0);
    assert((producer->contents & FRAME_BODY_RETURN) != 0 && (producer->contents & FRAME_TARGET_PROTECTION) != 0);
    assert((producer->contents & FRAME_OUTPUT_CONTINUATION) != 0);
    assert(plan.entry->last->symbol->reference && plan.entry->last->type == plan.entry);
    for (const PlannedType *type = plan.first; type != NULL; type = type->next) {
        for (const StorageField *field = type->first; field != NULL; field = field->next) {
            if (field->form == STORAGE_OWNED) { assert(field->type->layout_ordinal < type->layout_ordinal); }
        }
    }
    assert(plan.shared_first->service_storage == 0 && plan.shared_first->immutable);
    destroy_plan();
}
/** Physical state is not elided merely because a body is empty or has no textual yield. */
static void it_retains_the_general_fallback_and_keeps_shared_namespaces_out_of_pools(void) {
    prepare_types();
    file("generated/program/packages/test/fixture/v1.0.0/src/palette", "@=\nseed\n/test/fixture/v1.0.0/src/value\n |1\n");
    PlanStatus status = plan_text("@=\nshared\n/test/fixture/v1.0.0/src/value\n /test/fixture/v1.0.0/src/palette/seed\n@!\nvalue\n/test/fixture/v1.0.0/src/value\n");
    assert(status == PLAN_OK && plan.valid);
    assert(find_control(plan.entry, CONTROL_COMPLETE) != NULL);
    PlannedType *palette = program_plan_type(&plan, semantic_find_type(&semantic, "/test/fixture/v1.0.0/src/palette"));
    PlannedType *value = program_plan_type(&plan, semantic_find_type(&semantic, "/test/fixture/v1.0.0/src/value"));
    assert(!palette->requires_pool && !palette->stable_slots && palette->service_storage == 0);
    assert(value->stable_slots && (value->service_storage & STORAGE_POSITION) != 0);
    assert(plan.context_storage.pools == 2 && plan.context_storage.constants == 2);
    size_t before = allocation_count;
    bool valid = program_plan_validate(&plan);
    assert(valid && allocation_count == before);
    destroy_plan();
}
/** Separate plan owners have identical logical ordinals and borrowed identities while sharing no mutable planned records. */
static void it_preserves_deterministic_storage_across_independent_plan_owners(void) {
    prepare_storage(storage_main);
    bool built = plan_storage(&plan);
    assert(built);
    ProgramPlan second;
    program_plan_init(&second, &semantic, sources.allocator);
    built = plan_storage(&second);
    assert(built && second.first != plan.first && second.entry->frames != plan.entry->frames);
    assert(second.context_storage.pools == plan.context_storage.pools);
    assert(second.context_storage.constants == plan.context_storage.constants);
    const PlannedType *other = second.first;
    for (const PlannedType *type = plan.first; type != NULL; type = type->next, other = other->next) {
        assert(other != NULL && other != type && other->type == type->type);
        assert(other->layout_ordinal == type->layout_ordinal && other->field_count == type->field_count);
        assert(other->service_storage == type->service_storage && other->frame_count == type->frame_count);
        assert(other->cell_count == type->cell_count && other->origin_count == type->origin_count);
        const StorageFrame *right = other->frames;
        for (const StorageFrame *left = type->frames; left != NULL; left = left->next, right = right->next) {
            assert(right != NULL && right != left && right->statement == left->statement);
            assert(right->slot == left->slot && right->contents == left->contents && right->kind == left->kind);
            assert((right->operation_parent == NULL) == (left->operation_parent == NULL));
            if (left->operation_parent != NULL) { assert(left->operation_parent->slot == right->operation_parent->slot); }
            assert((right->traversal_parent == NULL) == (left->traversal_parent == NULL));
            if (left->traversal_parent != NULL) { assert(left->traversal_parent->slot == right->traversal_parent->slot); }
        }
        assert(right == NULL);
    }
    assert(other == NULL);
    program_plan_destroy(&second);
    destroy_plan();
}
