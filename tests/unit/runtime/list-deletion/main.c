/** @file
 * @brief Verify atomic deletion preflight and terminal exact-credit cleanup.
 */
#include "common.c"

/** A busy child in the second entry blocks the entire clear before the first
 * native destructor or membership retirement, including protected-prefix rollback.
 */
static void it_refuses_clear_before_any_effect_when_a_later_owned_descendant_is_busy(void) {
    setup_deletion(true);
    AmaProgramListService *service = open_insertion(1);
    AmaHandle first = *ama_handle_list_element(service->list, 0);
    AmaHandle second = *ama_handle_list_element(service->list, 1);
    AmaPoolCell *busy = NULL, *prefix = NULL;
    AmaStatus status = ama_program_cell(&program, deletion_child(second), &busy);
    assert(status.code == AMA_OK);
    status = ama_program_cell(&program, deletion_child(first), &prefix);
    assert(status.code == AMA_OK);
    busy->metadata.service.activity = AMA_INSTANCE_RUNNING;
    AmaMembership identity;
    AmaHandle target;
    status = ama_membership_at(service->list, 0, &identity, &target);
    assert(status.code == AMA_OK);
    const AmaProgramListInputs plan = {.integer_type = 3};
    AmaProgramListMethod *method = &owner_value(0)->methods[0];
    bool refused;
    status = ama_program_list_clear(service, method, &plan, &refused);
    assert(status.code == AMA_BUSY && refused && drops == 0 && service->list->count == 2);
    assert(prefix->metadata.service.activity == AMA_INSTANCE_READY && !world.failed);
    const AmaMembershipRecord *record = NULL;
    status = ama_membership_lookup(service->list, identity, &record);
    assert(status.code == AMA_OK && record->position == 0);
    end_insertion(method);
    busy->metadata.service.activity = AMA_INSTANCE_READY;
    close_insertion(service);
    finish_deletion();
}

/** Remove only its selected tree, preserve the other busy entry, then clear it
 * after availability returns. Integer input is literal zero, never default 999.
 */
static void it_removes_only_the_selected_tree_and_retires_exact_memberships(void) {
    setup_deletion(true);
    AmaProgramListService *service = open_insertion(1);
    AmaMembership first_identity, second_identity;
    AmaHandle first, second;
    AmaStatus status = ama_membership_at(service->list, 0, &first_identity, &first);
    assert(status.code == AMA_OK);
    status = ama_membership_at(service->list, 1, &second_identity, &second);
    assert(status.code == AMA_OK);
    AmaPoolCell *busy = NULL;
    status = ama_program_cell(&program, deletion_child(second), &busy);
    assert(status.code == AMA_OK);
    busy->metadata.service.activity = AMA_INSTANCE_RUNNING;
    AmaProgramListMethod *method = &owner_value(0)->methods[0];
    const AmaProgramListInputs plan = {.integer_type = 3};
    bool refused;
    status = ama_program_list_remove(service, method, &plan, &refused);
    assert(status.code == AMA_OK && !refused && service->list->count == 1 && drops == 1);
    assert(integer_defaults == 0 && integer_literals == 1 && index_calls == 1);
    assert(busy->metadata.service.activity == AMA_INSTANCE_RUNNING);
    const AmaMembershipRecord *record = NULL;
    status = ama_membership_lookup(service->list, first_identity, &record);
    assert(status.code == AMA_STALE_REFERENCE);
    status = ama_membership_lookup(service->list, second_identity, &record);
    assert(status.code == AMA_OK && record->position == 0 && ama_handle_equal(record->target, second));
    AmaPoolCell *removed = NULL;
    status = ama_program_cell(&program, first, &removed);
    assert(status.code == AMA_STALE_REFERENCE);
    end_insertion(method);
    assert(integer_drops == 1);
    busy->metadata.service.activity = AMA_INSTANCE_READY;
    status = ama_program_list_clear(service, method, &plan, &refused);
    assert(status.code == AMA_OK && !refused && service->list->count == 0 && drops == 2);
    assert(integer_literals == 1 && index_calls == 1 && !method->has_result);
    end_insertion(method);
    close_insertion(service);
    finish_deletion();
}

/** Duplicate references retire separately; clear never acquires a surviving busy
 * target, and an empty clear constructs/converts no irrelevant integer parameter.
 */
static void it_clears_reference_occurrences_without_acquiring_or_destroying_targets(void) {
    setup_deletion(false);
    result_type.hooks.initialize = result_default;
    AmaHandle target;
    AmaStatus status = ama_object_create(&world, 1, NULL, &target);
    assert(status.code == AMA_OK);
    result_type.hooks.initialize = insertion_busy_constructor;
    AmaPoolCell *cell = NULL;
    status = ama_program_cell(&program, target, &cell);
    assert(status.code == AMA_OK);
    cell->metadata.service.activity = AMA_INSTANCE_RUNNING;
    AmaMembership first, second;
    status = ama_membership_insert(&owner_value(0)->items, 0, target, &first);
    assert(status.code == AMA_OK);
    status = ama_membership_insert(&owner_value(0)->items, 1, target, &second);
    assert(status.code == AMA_OK && first.record != second.record);
    AmaProgramListService *service = open_insertion(0);
    AmaProgramListMethod *method = &owner_value(0)->methods[0];
    const AmaProgramListInputs plan = {.integer_type = 3};
    for (size_t iteration = 0; iteration < 2; iteration++) {
        bool refused;
        status = ama_program_list_clear(service, method, &plan, &refused);
        assert(status.code == AMA_OK && !refused && service->list->count == 0 && drops == 0);
        assert(cell->metadata.service.activity == AMA_INSTANCE_RUNNING);
        assert(integer_defaults == 0 && integer_literals == 0 && index_calls == 0);
        end_insertion(method);
    }
    const AmaMembershipRecord *record = NULL;
    status = ama_membership_lookup(service->list, first, &record);
    assert(status.code == AMA_STALE_REFERENCE);
    status = ama_membership_lookup(service->list, second, &record);
    assert(status.code == AMA_STALE_REFERENCE);
    close_insertion(service);
    cell->metadata.service.activity = AMA_INSTANCE_READY;
    status = ama_object_destroy(&world, target);
    assert(status.code == AMA_OK && drops == 1);
    finish_deletion();
}

/** Destruction failure consumes every removed credit and remains terminal;
 * retrying context cleanup cannot call these native destructors again.
 */
static void it_finishes_terminal_clear_cleanup_without_reclassifying_or_repeating_destruction(void) {
    setup_deletion(true);
    AmaProgramListService *service = open_insertion(1);
    AmaProgramListMethod *method = &owner_value(0)->methods[0];
    const AmaProgramListInputs plan = {.integer_type = 3};
    destructor_failure = true;
    bool refused = true;
    AmaStatus status = ama_program_list_clear(service, method, &plan, &refused);
    assert(status.code == AMA_IO_FAILURE && !refused && world.failed && drops == 2);
    assert(service->list->count == 0 && !method->has_result);
    end_insertion(method);
    close_insertion(service);
    finish_deletion();
    assert(drops == 2);
}

/** Visit every allocation needed by clear preflight. No failed prefix may retire
 * an entry or consume a native credit; cleanup can still release the exact grant.
 */
static void it_preserves_the_full_forest_after_each_clear_preflight_allocation_failure(void) {
    for (size_t ordinal = 1;; ordinal++) {
        setup_deletion(true);
        AmaProgramListService *service = open_insertion(1);
        AmaProgramListMethod *method = &owner_value(0)->methods[0];
        const AmaProgramListInputs plan = {.integer_type = 3};
        failure_at = allocation_count + ordinal;
        bool refused;
        AmaStatus status = ama_program_list_clear(service, method, &plan, &refused);
        bool success = status.code == AMA_OK;
        assert(!refused && (success || status.code == AMA_OUT_OF_MEMORY));
        assert(service->list->count == (success ? 0u : 2u) && drops == (success ? 2u : 0u));
        failure_at = 0;
        end_insertion(method);
        close_insertion(service);
        finish_deletion();
        if (success) { break; }
    }
}

/** Run whole-forest, selected-tree, nonowning and failure cleanup laws explicitly. */
void run(void) {
    it_refuses_clear_before_any_effect_when_a_later_owned_descendant_is_busy();
    it_removes_only_the_selected_tree_and_retires_exact_memberships();
    it_clears_reference_occurrences_without_acquiring_or_destroying_targets();
    it_finishes_terminal_clear_cleanup_without_reclassifying_or_repeating_destruction();
    it_preserves_the_full_forest_after_each_clear_preflight_allocation_failure();
}
/** Return success only after all real native resources and pool blocks are released. */
int main(void) { run(); return EXIT_SUCCESS; }
