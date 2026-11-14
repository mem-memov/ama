/** @file
 * @brief Exercise stable swap identities and exact fresh size-result lifetimes.
 */
#include "../list-insertion/common.c"
#include "runtime/program/lists/ordering/ordering.h"
static size_t size_calls;
static AmaStatus size_status;
static AmaHandle ordering_target;

/** Construct once in final storage; failure leaves no live native credit. */
static AmaStatus integer_size(void *storage, size_t value) {
    size_calls++;
    return size_status.code == AMA_OK ? integer_construct(storage, value, false) : size_status;
}
/** Disable element constructors/clones after seeding nonowning entries. */
static AmaProgramListService *setup_ordering(void) {
    setup_insertion();
    size_calls = 0;
    size_status = ama_status_success();
    AmaStatus status = ama_object_create(&world, 1, NULL, &ordering_target);
    assert(status.code == AMA_OK);
    AmaPoolCell *target_cell = NULL;
    status = ama_program_cell(&program, ordering_target, &target_cell);
    assert(status.code == AMA_OK);
    target_cell->metadata.service.activity = AMA_INSTANCE_RUNNING;
    result_type.hooks.initialize = insertion_busy_constructor;
    result_type.hooks.clone = insertion_busy_clone;
    AmaMembership identity;
    status = ama_membership_insert(&owner_value(0)->items, 0, ordering_target, &identity);
    assert(status.code == AMA_OK);
    status = ama_membership_insert(&owner_value(0)->items, 1, (AmaHandle){0}, &identity);
    assert(status.code == AMA_OK);
    return open_insertion(0);
}
/** Release the independent target after all structural grants have ended. */
static void finish_ordering(void) {
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_program_cell(&program, ordering_target, &cell);
    assert(status.code == AMA_OK);
    cell->metadata.service.activity = AMA_INSTANCE_READY;
    status = ama_object_destroy(&world, ordering_target);
    assert(status.code == AMA_OK);
    finish_insertion();
}
/** Both conversions succeed before order changes; target state is irrelevant. */
static void it_swaps_memberships_without_touching_targets_and_preserves_self_swap(void) {
    AmaProgramListService *service = setup_ordering();
    AmaMembership identities[2];
    AmaHandle targets[2];
    for (size_t index = 0; index < 2; index++) {
        AmaStatus status = ama_membership_at(service->list, index, &identities[index], &targets[index]);
        assert(status.code == AMA_OK);
    }
    const unsigned char one[] = {'1'};
    const AmaProgramInput input = {.field = 1, .source = {.kind = AMA_SOURCE_LITERAL, .literal = {one, 1}}};
    const AmaProgramListInputs plan = {.inputs = &input, .count = 1, .integer_type = 3};
    AmaProgramListMethod *method = &owner_value(0)->methods[0];
    bool refused;
    AmaStatus status = ama_program_list_swap(service, method, &plan, &refused);
    assert(status.code == AMA_OK && !refused && !method->has_result);
    assert(integer_literals == 2 && integer_defaults == 0 && index_calls == 2 && drops == 0);
    for (size_t index = 0; index < 2; index++) {
        const AmaMembershipRecord *record = NULL;
        status = ama_membership_lookup(service->list, identities[index], &record);
        assert(status.code == AMA_OK && record->position == 1 - index && ama_handle_equal(record->target, targets[index]));
    }
    end_insertion(method);
    const AmaProgramListInputs self = {.integer_type = 3};
    status = ama_program_list_swap(service, method, &self, &refused);
    assert(status.code == AMA_OK && !refused && integer_drops == 2 && index_calls == 4);
    end_insertion(method);
    close_insertion(service);
    finish_ordering();
}
/** Reject either position with no order change; equal numeric native Busy is technical. */
static void it_rejects_either_swap_position_before_publication(void) {
    for (size_t ordinal = 0; ordinal < 2; ordinal++) {
        AmaProgramListService *service = setup_ordering();
        const unsigned char invalid[] = {'2'};
        AmaProgramInput input = {.field = ordinal, .source = {.kind = AMA_SOURCE_LITERAL, .literal = {invalid, 1}}};
        AmaProgramListInputs plan = {.inputs = &input, .count = 1, .integer_type = 3};
        AmaProgramListMethod *method = &owner_value(0)->methods[0];
        bool refused;
        AmaStatus status = ama_program_list_swap(service, method, &plan, &refused);
        assert(status.code == AMA_INDEX_BOUNDS && refused && index_calls == 2);
        assert(ama_handle_equal(*ama_handle_list_element(service->list, 0), ordering_target));
        end_insertion(method);
        index_status = ama_status_make(AMA_BUSY, NULL, 0);
        status = ama_program_list_swap(service, method, &plan, &refused);
        assert(status.code == AMA_BUSY && !refused && index_calls == 3);
        end_insertion(method);
        close_insertion(service);
        finish_ordering();
    }
}
/** Exact length uses one size constructor, no default/literal/index, and its
 * escaped reference becomes stale after the complete method lifetime.
 */
static void it_constructs_length_once_and_retires_its_owned_result(void) {
    AmaProgramListService *service = setup_ordering();
    const AmaProgramListInputs plan = {.integer_type = 3, .from_size = integer_size};
    AmaProgramListMethod *method = &owner_value(0)->methods[0];
    bool refused;
    AmaStatus status = ama_program_list_length(service, method, &plan, &refused);
    assert(status.code == AMA_OK && !refused && method->owns_result && method->has_result);
    AmaHandle escaped = method->value.handle;
    AmaPoolCell *cell = NULL;
    status = ama_program_cell(&program, escaped, &cell);
    assert(status.code == AMA_OK);
    const InsertionInteger *value = ama_pool_payload(&integer_type.pool, cell);
    assert(value->number == 2 && value->address == value);
    assert(size_calls == 1 && integer_defaults == 0 && integer_literals == 0 && index_calls == 0);
    end_insertion(method);
    status = ama_program_cell(&program, escaped, &cell);
    assert(status.code == AMA_STALE_REFERENCE && integer_drops == 1);
    size_status = ama_status_make(AMA_BUSY, NULL, 0);
    status = ama_program_list_length(service, method, &plan, &refused);
    assert(status.code == AMA_BUSY && !refused && !method->has_result && integer_drops == 1 && size_calls == 2);
    end_insertion(method);
    close_insertion(service);
    finish_ordering();
}
/** The same selected size recipe round-trips boundaries through SIZE_MAX with
 * no fabricated list or impossible allocation of SIZE_MAX entries.
 */
static void it_constructs_full_host_size_boundaries_without_literal_round_trips(void) {
    AmaProgramListService *service = setup_ordering();
    const size_t values[] = {0, 1, (size_t)INT_MAX, (size_t)INT_MAX + 1, SIZE_MAX / 2, SIZE_MAX};
    for (size_t index = 0; index < sizeof(values) / sizeof(*values); index++) {
        AmaProgramListMethod *method = &owner_value(0)->methods[0];
        AmaStatus status = ama_program_list_method_begin(service, method);
        assert(status.code == AMA_OK);
        const AmaRecipe recipe = {.constructor = AMA_CONSTRUCT_SIZE, .size_value = values[index], .from_size = integer_size};
        status = ama_program_list_owned_result(method, 3, &recipe);
        assert(status.code == AMA_OK);
        AmaPoolCell *cell = NULL;
        status = ama_program_cell(&program, method->value.handle, &cell);
        assert(status.code == AMA_OK);
        size_t converted = 0;
        status = integer_index(ama_pool_payload(&integer_type.pool, cell), &converted);
        assert(status.code == AMA_OK && converted == values[index]);
        end_insertion(method);
    }
    assert(size_calls == 6 && integer_defaults == 0 && integer_literals == 0 && integer_drops == 6);
    close_insertion(service);
    finish_ordering();
}
/** Run ordering identity, refusal provenance and constructor boundary contracts. */
void run(void) {
    it_swaps_memberships_without_touching_targets_and_preserves_self_swap();
    it_rejects_either_swap_position_before_publication();
    it_constructs_length_once_and_retires_its_owned_result();
    it_constructs_full_host_size_boundaries_without_literal_round_trips();
}
/** Return only after all actual integer resource credits have been retired. */
int main(void) { run(); return EXIT_SUCCESS; }
