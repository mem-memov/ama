/** @file
 * @brief Compare fixed-scan publication against independent numerical boundaries.
 */
#include "common.c"
#include "runtime/context/internal.h"
#include "destruction.c"

/** Exhaust all positions and two independent boundaries in small actual lists;
 * insertion at the gap belongs to the future, and only strict past insertion moves q.
 */
static void it_corrects_both_nested_boundaries_for_every_insertion_position(void) {
    for (size_t count = 0; count <= 4; count++) {
        for (size_t first = 0; first <= count; first++) {
            for (size_t second = 0; second <= count; second++) {
                for (size_t position = 0; position <= count; position++) {
                    setup_correction(count);
                    (void)advance_correction(0, first);
                    (void)advance_correction(1, second);
                    AmaListStorage storage = correction_storage();
                    checked(ama_list_storage_insert(&storage, position, owners[1]));
                    assert(owner_value(0)->cursors[0].boundary == first + (position < first)
                        && owner_value(0)->cursors[1].boundary == second + (position < second));
                    assert(storage.list->count == count + 1 && storage.list->acquisition == NULL);
                    finish_correction();
                }
            }
        }
    }
}
/** Count removed past positions explicitly as an independent finite-set oracle,
 * rather than repeating the implementation's subtraction/minimum formula.
 */
static size_t expected_removal(size_t boundary, size_t position, size_t count) {
    size_t remaining = 0;
    for (size_t index = 0; index < boundary; index++) {
        if (index < position || index - position >= count) { remaining++; }
    }
    return remaining;
}
/** Exhaust complete removal ranges including empty edits, full clear and ranges
 * that cross one boundary but are wholly past/future for the other cursor.
 */
static void it_corrects_both_nested_boundaries_for_every_removed_range(void) {
    for (size_t length = 0; length <= 4; length++) {
        for (size_t first = 0; first <= length; first++) {
            for (size_t second = 0; second <= length; second++) {
                for (size_t position = 0; position <= length; position++) {
                    for (size_t count = 0; count <= length - position; count++) {
                        setup_correction(length);
                        (void)advance_correction(0, first);
                        (void)advance_correction(1, second);
                        AmaListStorage storage = correction_storage();
                        checked(ama_list_storage_remove(&storage, position, count));
                        assert(owner_value(0)->cursors[0].boundary == expected_removal(first, position, count)
                            && owner_value(0)->cursors[1].boundary == expected_removal(second, position, count));
                        assert(storage.list->count == length - count);
                        finish_correction();
                    }
                }
            }
        }
    }
}
/** Clear/append and remove-current/reinsert actually select the newly published
 * membership; an exhausted cursor is not reopened by later append.
 */
static void it_visits_new_memberships_at_the_corrected_gap_and_never_reopens_closed_scans(void) {
    setup_correction(2);
    AmaProgramForeachPlan plan = advance_correction(0, 1);
    AmaMembership original = owner_value(0)->cursors[0].current;
    AmaListStorage storage = correction_storage();
    checked(ama_list_storage_remove(&storage, 0, 1));
    checked(ama_list_storage_insert(&storage, 0, owners[1]));
    bool selected;
    checked(ama_program_foreach_next(&program, owners[0], &plan, &selected));
    assert(selected && owner_value(0)->cursors[0].boundary == 1
        && (owner_value(0)->cursors[0].current.record != original.record
            || owner_value(0)->cursors[0].current.generation != original.generation));
    checked(ama_list_storage_remove(&storage, 0, storage.list->count));
    assert(owner_value(0)->cursors[0].boundary == 0);
    checked(ama_list_storage_insert(&storage, 0, owners[1]));
    checked(ama_program_foreach_next(&program, owners[0], &plan, &selected));
    assert(selected && owner_value(0)->cursors[0].boundary == 1);
    checked(ama_program_foreach_next(&program, owners[0], &plan, &selected));
    assert(!selected && !owner_value(0)->cursors[0].active);
    checked(ama_list_storage_insert(&storage, 1, owners[1]));
    assert(!owner_value(0)->cursors[0].active);
    finish_correction();
}
/** Swap across the boundary repeats the prior membership and omits the moved
 * future membership without a hidden visited set or boundary adjustment.
 */
static void it_preserves_swap_boundaries_and_permits_positional_repeats_and_skips(void) {
    setup_correction(3);
    AmaProgramForeachPlan plan = advance_correction(0, 1);
    AmaMembership first = owner_value(0)->cursors[0].current;
    checked(ama_membership_swap(&owner_value(0)->items, 0, 2));
    assert(owner_value(0)->cursors[0].boundary == 1);
    bool selected;
    checked(ama_program_foreach_next(&program, owners[0], &plan, &selected));
    assert(selected && owner_value(0)->cursors[0].current.record != first.record);
    checked(ama_program_foreach_next(&program, owners[0], &plan, &selected));
    assert(selected && owner_value(0)->cursors[0].current.record == first.record);
    checked(ama_program_foreach_next(&program, owners[0], &plan, &selected));
    assert(!selected);
    finish_correction();
}
/** Old incarnations and inactive sites are never corrected; malformed duplicate
 * catalogs or invalid active boundaries fail before the first structural effect.
 */
static void it_ignores_unrelated_slots_and_rejects_invalid_catalogs_before_mutation(void) {
    setup_correction(2);
    (void)advance_correction(0, 1);
    (void)advance_correction(1, 2);
    owner_value(0)->cursors[1].incarnation++;
    AmaListStorage storage = correction_storage();
    checked(ama_list_storage_insert(&storage, 0, owners[1]));
    assert(owner_value(0)->cursors[0].boundary == 2 && owner_value(0)->cursors[1].boundary == 2);
    static const size_t duplicate_offsets[] = {offsetof(TraversalOwner, cursors), offsetof(TraversalOwner, cursors)};
    static const AmaListCursorSet invalid_sets[] = {{duplicate_offsets, 2}, {0}, {0}};
    owner_type.list_cursors = invalid_sets;
    AmaStatus status = ama_list_storage_remove(&storage, 0, 1);
    assert(status.code == AMA_INVALID_STATE && storage.list->count == 3
        && owner_value(0)->cursors[0].boundary == 2 && !world.operation_active);
    owner_type.list_cursors = reference_cursors;
    owner_value(0)->cursors[0].boundary = 4;
    status = ama_list_storage_insert(&storage, 0, owners[1]);
    assert(status.code == AMA_INVALID_STATE && storage.list->count == 3);
    owner_value(0)->cursors[0].boundary = 2;
    finish_correction();
}
/** A busy owning subtree refuses before any deletion or boundary correction;
 * successful removal afterwards corrects both real active scans exactly once.
 */
static void it_preserves_all_boundaries_when_owning_deletion_preflight_refuses(void) {
    setup_traversal();
    owner_type.list_cursors = owned_cursors;
    AmaListStorage storage = {&world, owners[0], &owner_value(0)->owned, 1, true};
    checked(ama_list_storage_insert(&storage, 0, (AmaHandle){0}));
    checked(ama_list_storage_insert(&storage, 1, (AmaHandle){0}));
    AmaProgramForeachPlan plans[2];
    for (size_t site = 0; site < 2; site++) {
        plans[site] = traversal_plan(site, false);
        plans[site].field = 2;
        checked(ama_program_foreach_enter(&program, owners[0], &plans[site]));
        for (size_t index = 0; index <= site; index++) {
            bool selected;
            checked(ama_program_foreach_next(&program, owners[0], &plans[site], &selected));
            assert(selected);
        }
    }
    AmaPoolCell *busy = NULL;
    checked(ama_program_cell(&program, *ama_handle_list_element(storage.list, 1), &busy));
    busy->metadata.service.activity = AMA_INSTANCE_RUNNING;
    bool refused;
    AmaStatus status = ama_list_storage_remove_checked(&storage, 0, 2, &refused);
    assert(status.code == AMA_BUSY && refused && storage.list->count == 2
        && owner_value(0)->cursors[0].boundary == 1 && owner_value(0)->cursors[1].boundary == 2);
    busy->metadata.service.activity = AMA_INSTANCE_READY;
    checked(ama_list_storage_remove(&storage, 0, 1));
    assert(owner_value(0)->cursors[0].boundary == 0 && owner_value(0)->cursors[1].boundary == 1);
    for (size_t site = 0; site < 2; site++) { checked(ama_program_foreach_leave(&program, owners[0], &plans[site], false)); }
    finish_traversal();
}
/** Reserved metadata failure cannot publish either an insertion or a correction. */
static void it_preserves_boundaries_after_insertion_preparation_allocation_failure(void) {
    setup_correction(8);
    (void)advance_correction(0, 1);
    (void)advance_correction(1, 8);
    AmaListStorage storage = correction_storage();
    /* Fill the currently reserved order extent so the next insertion must grow. */
    while (storage.list->count < storage.list->capacity) {
        checked(ama_list_storage_insert(&storage, storage.list->count, owners[1]));
    }
    size_t length = storage.list->count;
    failure_at = allocation_count + 1;
    AmaStatus status = ama_list_storage_insert(&storage, 0, owners[1]);
    assert(status.code == AMA_OUT_OF_MEMORY && storage.list->count == length
        && owner_value(0)->cursors[0].boundary == 1 && owner_value(0)->cursors[1].boundary == 8);
    failure_at = 0;
    finish_correction();
}
/** Context factories share only immutable catalog offsets, never mutable cursor
 * state; malformed catalogs fail validation before allocating descriptors/roots.
 */
static void it_validates_and_installs_catalogs_in_independent_contexts(void) {
    setup_test();
    static const AmaObjectField fields[] = {
        {AMA_FIELD_REFERENCE_LIST, 1, offsetof(TraversalOwner, items)},
        {AMA_FIELD_REFERENCE, 1, offsetof(TraversalOwner, reused)},
        {AMA_FIELD_OWNING_LIST, 1, offsetof(TraversalOwner, owned)}
    };
    static const AmaProgramField declarations[] = {
        {.role = AMA_PROGRAM_STATE}, {.role = AMA_PROGRAM_STATE}, {.role = AMA_PROGRAM_STATE}
    };
    AmaContextType schema = {.type_id = 1, .size = sizeof(TraversalOwner), .alignment = _Alignof(TraversalOwner),
        .slots_per_block = 2, .generation_limit = UINT64_MAX, .fields = fields, .declarations = declarations,
        .field_count = 3, .native_offset = offsetof(TraversalOwner, payload), .list_cursors = reference_cursors};
    const AmaContextDefinition definition = {.types = &schema, .type_count = 1};
    checked(ama_context_validate(&definition));
    AmaContext contexts[2] = {0};
    TraversalOwner *values[2];
    for (size_t index = 0; index < 2; index++) {
        checked(ama_context_prepare(&contexts[index], &definition, sources.allocator));
        AmaObjectType *type = contexts[index].world.types[0];
        assert(type->list_cursors == reference_cursors && type->pool.live_count == 0);
        AmaHandle root;
        checked(ama_object_create(&contexts[index].world, 1, NULL, &root));
        AmaPoolCell *cell = NULL;
        checked(ama_pool_lookup(&type->pool, root, false, &cell));
        values[index] = ama_pool_payload(&type->pool, cell);
    }
    assert(values[0] != values[1]);
    values[0]->cursors[0].active = true;
    assert(!values[1]->cursors[0].active);
    for (size_t index = 0; index < 2; index++) { checked(ama_context_cleanup(&contexts[index], ama_status_success())); }
    static const size_t duplicates[] = {offsetof(TraversalOwner, cursors), offsetof(TraversalOwner, cursors)};
    static const AmaListCursorSet invalid[] = {{duplicates, 2}, {0}, {0}};
    schema.list_cursors = invalid;
    size_t allocations = allocation_count;
    AmaStatus status = ama_context_validate(&definition);
    assert(status.code == AMA_INVALID_STATE && allocation_count == allocations);
    destroy_test();
}
/** Irreversible terminal destruction still removes the complete range and
 * corrects both active cursors before releasing the publication guard.
 */
static void it_corrects_every_boundary_after_terminal_native_clear_failure(void) {
    setup_correction_destruction();
    AmaListStorage storage = {&world, owners[0], &owner_value(0)->owned, 2, true};
    for (size_t index = 0; index < 2; index++) { checked(ama_list_storage_insert(&storage, index, (AmaHandle){0})); }
    AmaProgramForeachPlan plans[2];
    for (size_t site = 0; site < 2; site++) {
        plans[site] = traversal_plan(site, true);
        plans[site].field = 2;
        plans[site].element_type = 2;
        checked(ama_program_foreach_enter(&program, owners[0], &plans[site]));
        for (size_t index = 0; index <= site; index++) {
            bool selected;
            checked(ama_program_foreach_next(&program, owners[0], &plans[site], &selected));
            assert(selected);
        }
    }
    correction_destroy_failure = true;
    bool refused = true;
    AmaStatus status = ama_list_storage_remove_checked(&storage, 0, 2, &refused);
    assert(status.code == AMA_BUSY && !refused && world.failed && correction_drops == 2
        && storage.list->count == 0 && owner_value(0)->cursors[0].boundary == 0
        && owner_value(0)->cursors[1].boundary == 0 && !world.operation_active);
    for (size_t site = 0; site < 2; site++) { checked(ama_program_foreach_leave(&program, owners[0], &plans[site], false)); }
    ama_pool_dispose(&correction_element_type.pool);
    finish_traversal();
    assert(correction_drops == 2);
}
/** Real yield preserves cursor slots; an ordinary read acquisition by another
 * caller mutates the yielded owner's list and corrects both suspended boundaries.
 */
static void it_corrects_suspended_boundaries_through_another_callers_list_service(void) {
    setup_correction(2);
    (void)advance_correction(0, 1);
    (void)advance_correction(1, 2);
    AmaPoolCell *suspended = NULL, *caller = NULL;
    checked(ama_program_cell(&program, owners[0], &suspended));
    checked(ama_program_cell(&program, owners[1], &caller));
    suspended->metadata.service.resume = (AmaResumePosition){.valid = true, .selector = 41};
    suspended->metadata.service.return_link = (AmaReturnLink){.attached = true, .host = true};
    uint64_t continuation = UINT64_MAX;
    checked(ama_program_yield(&program, &program.current, &continuation));
    assert(continuation == 0 && suspended->metadata.service.activity == AMA_INSTANCE_YIELDED);
    program.current = owners[1];
    caller->metadata.service.activity = AMA_INSTANCE_RUNNING;
    owner_value(1)->reused = owners[0];
    const AmaProgramAccess access = {.field = 1};
    const AmaProgramInvocation invocation = {.target = &access, .mode = AMA_ACCESS_READ};
    bool refused;
    checked(ama_program_acquire(&program, owners[1], &invocation, &refused));
    assert(!refused);
    AmaProgramListService service = {0};
    checked(ama_program_list_open(&program, owners[1], owners[0], 0, &service, &refused));
    AmaListStorage storage = correction_storage();
    checked(ama_list_storage_insert(&storage, 0, owners[1]));
    assert(owner_value(0)->cursors[0].boundary == 2 && owner_value(0)->cursors[1].boundary == 3);
    checked(ama_program_list_close(&service));
    checked(ama_program_release(&program, owners[1]));
    assert(suspended->metadata.service.activity == AMA_INSTANCE_YIELDED
        && suspended->metadata.service.resume.valid && suspended->metadata.service.resume.selector == 41);
    caller->metadata.service.activity = AMA_INSTANCE_READY;
    suspended->metadata.service.resume = (AmaResumePosition){0};
    program.current = owners[0];
    finish_correction();
}
/** Run actual mutation publication against exact small-state oracles. */
void run(void) {
    it_corrects_both_nested_boundaries_for_every_insertion_position();
    it_corrects_both_nested_boundaries_for_every_removed_range();
    it_visits_new_memberships_at_the_corrected_gap_and_never_reopens_closed_scans();
    it_preserves_swap_boundaries_and_permits_positional_repeats_and_skips();
    it_ignores_unrelated_slots_and_rejects_invalid_catalogs_before_mutation();
    it_preserves_all_boundaries_when_owning_deletion_preflight_refuses();
    it_preserves_boundaries_after_insertion_preparation_allocation_failure();
    it_validates_and_installs_catalogs_in_independent_contexts();
    it_corrects_every_boundary_after_terminal_native_clear_failure();
    it_corrects_suspended_boundaries_through_another_callers_list_service();
}
/** Return only after every real list, cursor and fixture allocation is released. */
int main(void) { run(); return EXIT_SUCCESS; }
