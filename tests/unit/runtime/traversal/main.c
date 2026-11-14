/** @file
 * @brief Check short scan acquisition, exact local activation and nested reuse.
 */
#include "common.c"

/** An empty parsed body checks only collection access, even with no usable
 * lexical slot offsets, and preserves both local/reused cells on nonempty data.
 */
static void it_validates_empty_bodies_without_activating_or_visiting_bindings(void) {
    setup_traversal();
    append_reference(owners[0]);
    AmaProgramForeachPlan plan = traversal_plan(0, true);
    plan.cursor_offset = plan.binding_offset = plan.value_offset = SIZE_MAX;
    checked(ama_program_foreach_empty(&program, owners[0], &plan));
    TraversalOwner *value = owner_value(0);
    assert(!value->cursors[0].active && value->binding.roles == 0
        && ama_handle_equal(value->reused, owners[1]) && value->items.acquisition == NULL);
    AmaProgramListService service = {0};
    bool refused;
    checked(ama_program_list_open(&program, owners[0], owners[0], 0, &service, &refused));
    AmaStatus status = ama_program_foreach_empty(&program, owners[0], &plan);
    assert(status.code == AMA_BUSY && value->items.acquisition == &service);
    checked(ama_program_list_close(&service));
    finish_traversal();
}
/** Zero candidates never activate a local or overwrite an existing reference. */
static void it_preserves_unactivated_bindings_for_empty_and_absent_only_lists(void) {
    for (size_t local = 0; local < 2; local++) {
        setup_traversal();
        AmaProgramForeachPlan plan = traversal_plan(0, local != 0);
        checked(ama_program_foreach_enter(&program, owners[0], &plan));
        bool has_value = true;
        checked(ama_program_foreach_next(&program, owners[0], &plan, &has_value));
        assert(!has_value && owner_value(0)->binding.roles == 0);
        append_reference((AmaHandle){0});
        AmaHandle stale = owners[1];
        checked(ama_object_destroy(&world, stale));
        checked(ama_object_create(&world, 1, NULL, &owners[1]));
        append_reference(stale);
        AmaHandle previous = owner_value(0)->reused;
        checked(ama_program_foreach_enter(&program, owners[0], &plan));
        checked(ama_program_foreach_next(&program, owners[0], &plan, &has_value));
        assert(!has_value && ama_handle_equal(owner_value(0)->reused, previous));
        if (local != 0) {
            AmaProgramValue value;
            AmaStatus status = ama_program_foreach_value(&program, owners[0], &plan, &value);
            assert(status.code == AMA_INVALID_STATE);
        }
        finish_traversal();
    }
}
/** The live running caller is eligible twice as two distinct occurrences;
 * selection allocates nothing, retains no grant and leaves normal calls Busy.
 */
static void it_selects_busy_live_duplicates_without_acquiring_targets_or_allocating(void) {
    setup_traversal();
    append_reference((AmaHandle){0});
    append_reference(owners[0]);
    append_reference(owners[0]);
    AmaProgramForeachPlan plan = traversal_plan(0, true);
    size_t allocations = allocation_count;
    failure_at = allocation_count + 1;
    checked(ama_program_foreach_enter(&program, owners[0], &plan));
    AmaMembership first = {0};
    for (size_t iteration = 0; iteration < 2; iteration++) {
        bool has_value;
        checked(ama_program_foreach_next(&program, owners[0], &plan, &has_value));
        TraversalOwner *owner = owner_value(0);
        assert(has_value && owner->cursors[0].boundary == iteration + 2
            && owner->items.acquisition == NULL && owner->binding.roles == 1);
        if (iteration == 0) { first = owner->binding.origin; }
        else { assert(first.record != owner->binding.origin.record); }
        AmaProgramValue value;
        checked(ama_program_foreach_value(&program, owners[0], &plan, &value));
        assert(ama_handle_equal(value.handle, owners[0]));
        bool refused;
        AmaStatus status = ama_program_acquire_value(&program, owners[0], &value,
            AMA_ACCESS_ENTRY, offsetof(TraversalOwner, call_frame), &refused);
        assert(status.code == AMA_BUSY && refused);
    }
    bool has_value = true;
    checked(ama_program_foreach_next(&program, owners[0], &plan, &has_value));
    assert(!has_value && allocation_count == allocations);
    failure_at = 0;
    finish_traversal();
}
/** Inner reuse overwrites the shared reference/origin without restoring an
 * outer saved value; ending one role preserves the other's binding metadata.
 */
static void it_reuses_nested_binding_cells_without_shadowing_or_restoring_old_values(void) {
    setup_traversal();
    append_reference(owners[0]);
    append_reference(owners[1]);
    AmaProgramForeachPlan outer = traversal_plan(0, false), inner = traversal_plan(1, false);
    bool has_value;
    checked(ama_program_foreach_enter(&program, owners[0], &outer));
    checked(ama_program_foreach_next(&program, owners[0], &outer, &has_value));
    assert(has_value);
    checked(ama_program_foreach_enter(&program, owners[0], &inner));
    checked(ama_program_foreach_next(&program, owners[0], &inner, &has_value));
    assert(has_value && owner_value(0)->binding.roles == 2);
    checked(ama_program_foreach_next(&program, owners[0], &inner, &has_value));
    assert(has_value && ama_handle_equal(owner_value(0)->reused, owners[1]));
    AmaMembership origin = owner_value(0)->binding.origin;
    checked(ama_program_foreach_leave(&program, owners[0], &inner, false));
    assert(owner_value(0)->binding.roles == 1 && owner_value(0)->binding.associated
        && owner_value(0)->binding.origin.record == origin.record);
    checked(ama_program_foreach_leave(&program, owners[0], &outer, false));
    assert(!owner_value(0)->binding.associated && ama_handle_equal(owner_value(0)->reused, owners[1]));
    finish_traversal();
}
/** Whole-list incarnation change fails before installing any new binding, but
 * abandoning the obsolete cursor does not require reacquiring the old list.
 */
static void it_rejects_replaced_collections_and_still_releases_the_lexical_role(void) {
    setup_traversal();
    append_reference(owners[1]);
    AmaProgramForeachPlan plan = traversal_plan(0, true);
    checked(ama_program_foreach_enter(&program, owners[0], &plan));
    bool has_value;
    checked(ama_program_foreach_next(&program, owners[0], &plan, &has_value));
    assert(has_value);
    checked(ama_membership_reset(&owner_value(0)->items));
    AmaStatus status = ama_program_foreach_next(&program, owners[0], &plan, &has_value);
    assert(status.code == AMA_STALE_REFERENCE && !has_value && owner_value(0)->items.acquisition == NULL);
    checked(ama_program_foreach_leave(&program, owners[0], &plan, true));
    checked(ama_program_foreach_leave(&program, owners[0], &plan, true));
    finish_traversal();
}
/** An owning-list scan borrows the actual initialized child without cloning or
 * requiring its availability; the child remains owned by the original parent.
 */
static void it_selects_actual_owned_children_without_acquiring_or_copying_them(void) {
    setup_traversal();
    AmaListStorage storage = {&world, owners[0], &owner_value(0)->owned, 1, true};
    checked(ama_list_storage_insert(&storage, 0, (AmaHandle){0}));
    AmaHandle child = *ama_handle_list_element(storage.list, 0);
    AmaPoolCell *cell = NULL;
    checked(ama_program_cell(&program, child, &cell));
    cell->metadata.service.activity = AMA_INSTANCE_RUNNING;
    AmaProgramForeachPlan plan = traversal_plan(0, true);
    plan.field = 2;
    checked(ama_program_foreach_enter(&program, owners[0], &plan));
    bool has_value;
    checked(ama_program_foreach_next(&program, owners[0], &plan, &has_value));
    assert(has_value && ama_handle_equal(owner_value(0)->binding.installed, child)
        && ama_handle_equal(cell->owner, owners[0]) && cell->metadata.service.activity == AMA_INSTANCE_RUNNING
        && storage.list->acquisition == NULL);
    checked(ama_program_foreach_leave(&program, owners[0], &plan, false));
    cell->metadata.service.activity = AMA_INSTANCE_READY;
    finish_traversal();
}
/** Offset checks precede pointer arithmetic and acquiring the source list. */
static void it_rejects_invalid_lexical_slot_extents_before_touching_collection_state(void) {
    setup_traversal();
    for (size_t position = 0; position < 3; position++) {
        for (size_t invalid = 0; invalid < 2; invalid++) {
            AmaProgramForeachPlan plan = traversal_plan(0, true);
            size_t offset = invalid == 0 ? SIZE_MAX : 1;
            if (position == 0) { plan.cursor_offset = offset; }
            else if (position == 1) { plan.binding_offset = offset; }
            else { plan.value_offset = offset; }
            AmaStatus status = ama_program_foreach_enter(&program, owners[0], &plan);
            assert(status.code == AMA_INVALID_STATE && !owner_value(0)->cursors[0].active
                && owner_value(0)->items.acquisition == NULL && owner_value(0)->binding.roles == 0);
        }
    }
    finish_traversal();
}
/** Run exact activation, selection, reuse and invalidation contracts. */
void run(void) {
    it_validates_empty_bodies_without_activating_or_visiting_bindings();
    it_preserves_unactivated_bindings_for_empty_and_absent_only_lists();
    it_selects_busy_live_duplicates_without_acquiring_targets_or_allocating();
    it_reuses_nested_binding_cells_without_shadowing_or_restoring_old_values();
    it_rejects_replaced_collections_and_still_releases_the_lexical_role();
    it_selects_actual_owned_children_without_acquiring_or_copying_them();
    it_rejects_invalid_lexical_slot_extents_before_touching_collection_state();
}
/** Return only after every fixed role and counted pool allocation is released. */
int main(void) { run(); return EXIT_SUCCESS; }
