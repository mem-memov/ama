/** @file
 * @brief Run typed append/insert creation, provenance, bounds and allocation laws.
 */
#include "common.c"
#include <stdio.h>

/** Omission constructs initial state; supplied owned source clones independent
 * bytes, and result handles name the actual new stable element.
 */
static void it_appends_defaults_and_independent_owned_clones(void) {
    setup_insertion();
    AmaHandle source;
    AmaStatus status = ama_object_create(&world, 1, NULL, &source);
    assert(status.code == AMA_OK);
    AmaPoolCell *source_cell = NULL;
    status = ama_program_cell(&program, source, &source_cell);
    assert(status.code == AMA_OK);
    ServiceResult *original = ama_pool_payload(&result_type.pool, source_cell);
    memset(original->resource, 42, 8);
    owner_value(0)->source = source;
    AmaProgramListService *service = open_insertion(1);
    AmaProgramListMethod *method = &owner_value(0)->methods[0];
    const AmaProgramListInputs omitted = {.integer_type = 3, .operation = 42};
    bool refused;
    status = ama_program_list_append(service, method, &omitted, &refused);
    assert(status.code == AMA_OK && !refused && service->list->count == 1 && defaults == 2 && element_clones == 0);
    assert(ama_handle_equal(method->value.handle, *ama_handle_list_element(service->list, 0)));
    end_insertion(method);
    static const AmaProgramAccess access = {.field = 3};
    const AmaProgramInput input = {.field = 0, .source = {.kind = AMA_SOURCE_LOCAL, .access = &access, .type_id = 1}};
    const AmaProgramListInputs supplied = {.inputs = &input, .count = 1, .integer_type = 3, .operation = 43};
    status = ama_program_list_append(service, method, &supplied, &refused);
    assert(status.code == AMA_OK && !refused && service->list->count == 2 && defaults == 2 && element_clones == 1);
    AmaPoolCell *copy_cell = NULL;
    status = ama_program_cell(&program, method->value.handle, &copy_cell);
    assert(status.code == AMA_OK);
    ServiceResult *copy = ama_pool_payload(&result_type.pool, copy_cell);
    assert(copy->resource != original->resource && memcmp(copy->resource, original->resource, 8) == 0);
    end_insertion(method);
    close_insertion(service);
    status = ama_object_destroy(&world, source);
    assert(status.code == AMA_OK);
    finish_insertion();
}

/** Reference insertion copies busy/stale/empty handles without acquiring targets. */
static void it_appends_complete_reference_handles_without_cloning_targets(void) {
    setup_insertion();
    AmaHandle target;
    AmaStatus status = ama_object_create(&world, 1, NULL, &target);
    assert(status.code == AMA_OK);
    AmaPoolCell *cell = NULL;
    status = ama_program_cell(&program, target, &cell);
    assert(status.code == AMA_OK);
    cell->metadata.service.activity = AMA_INSTANCE_RUNNING;
    AmaProgramListService *service = open_insertion(0);
    static const AmaProgramAccess source = {.field = 3};
    const AmaProgramInput input = {.source = {.kind = AMA_SOURCE_LOCAL, .access = &source, .type_id = 1}};
    const AmaProgramListInputs plan = {.inputs = &input, .count = 1, .integer_type = 3};
    result_type.hooks.clone = NULL;
    AmaHandle handles[] = {target, {1, 999, 7}, {0}};
    for (size_t index = 0; index < 3; index++) {
        owner_value(0)->source = handles[index];
        bool refused;
        AmaProgramListMethod *method = &owner_value(0)->methods[0];
        status = ama_program_list_append(service, method, &plan, &refused);
        assert(status.code == AMA_OK && !refused && ama_handle_equal(method->value.handle, handles[index]));
        end_insertion(method);
    }
    assert(defaults == 1 && element_clones == 0 && drops == 0 && cell->metadata.service.activity == AMA_INSTANCE_RUNNING);
    close_insertion(service);
    cell->metadata.service.activity = AMA_INSTANCE_READY;
    status = ama_object_destroy(&world, target);
    assert(status.code == AMA_OK);
    finish_insertion();
}

/** Defaults use literal zero; reserved conversion/bounds failures refuse before
 * mutation, while the same Busy code from a converter remains technical.
 */
static void it_inserts_with_exact_position_conversion_and_preflight_provenance(void) {
    setup_insertion();
    AmaProgramListService *service = open_insertion(0);
    AmaProgramListMethod *method = &owner_value(0)->methods[0];
    AmaProgramListInputs plan = {.integer_type = 3};
    bool refused;
    AmaStatus status = ama_program_list_insert(service, method, &plan, &refused);
    assert(status.code == AMA_OK && !refused && service->list->count == 1);
    assert(integer_defaults == 0 && integer_literals == 1 && index_calls == 1);
    end_insertion(method);
    char maximum[3 * sizeof(size_t) + 2];
    int length = snprintf(maximum, sizeof(maximum), "%zu", SIZE_MAX);
    assert(length > 0 && (size_t)length < sizeof(maximum));
    const unsigned char *positions[] = {(const unsigned char *)"2", (const unsigned char *)"-1", (const unsigned char *)maximum};
    for (size_t index = 0; index < 3; index++) {
        const AmaProgramInput input = {.source = {.kind = AMA_SOURCE_LITERAL,
            .literal = {positions[index], strlen((const char *)positions[index])}}};
        plan.inputs = &input; plan.count = 1;
        status = ama_program_list_insert(service, method, &plan, &refused);
        assert(status.code == (index == 1 ? AMA_INVALID_INDEX : AMA_INDEX_BOUNDS) && refused && service->list->count == 1);
        assert(!method->has_result);
        end_insertion(method);
    }
    plan.inputs = NULL; plan.count = 0;
    AmaStatus failures[] = {ama_status_make(AMA_BUSY, NULL, 0),
        ama_status_make(AMA_OK, NULL, 1), ama_status_make(AMA_NATIVE_FAILURE, NULL, 0)};
    uint32_t expected[] = {AMA_BUSY, AMA_INVALID_STATE, AMA_NATIVE_FAILURE};
    for (size_t index = 0; index < 3; index++) {
        index_status = failures[index];
        status = ama_program_list_insert(service, method, &plan, &refused);
        assert(status.code == expected[index] && !refused && service->list->count == 1);
        end_insertion(method);
    }
    assert(integer_literals == integer_drops && integer_defaults == 0);
    close_insertion(service);
    finish_insertion();
}

/** An owned integer parameter copied from immutable storage gets an independent
 * native resource and retains exact protocol value, including insertion at n.
 */
static void it_copies_supplied_integer_inputs_and_inserts_at_the_end(void) {
    setup_insertion();
    AmaProgramListService *service = open_insertion(0);
    AmaProgramListMethod *method = &owner_value(0)->methods[0];
    AmaProgramListInputs plan = {.integer_type = 3};
    bool refused;
    AmaStatus status = ama_program_list_append(service, method, &plan, &refused);
    assert(status.code == AMA_OK && !refused);
    end_insertion(method);
    InsertionInteger original;
    status = integer_construct(&original, 1, false);
    assert(status.code == AMA_OK);
    AmaConstantValue value = {.type = &insertion_program_types[2], .payload = &original, .native_live = true};
    AmaConstantStorage storage = {.items = &value, .count = 1};
    AmaProgramConstant constant = {.storage = &storage, .type_id = 3};
    program.constants = &constant;
    program.constant_count = 1;
    const AmaProgramInput input = {.source = {.kind = AMA_SOURCE_SHARED, .shared = 0, .type_id = 3}};
    plan.inputs = &input; plan.count = 1;
    status = ama_program_list_insert(service, method, &plan, &refused);
    assert(status.code == AMA_OK && !refused && service->list->count == 2);
    assert(integer_clones == 1 && integer_defaults == 0 && integer_literals == 0 && index_calls == 1);
    AmaPoolCell *cell = NULL;
    status = ama_program_cell(&program, method->inputs[0], &cell);
    assert(status.code == AMA_OK);
    InsertionInteger *copy = ama_pool_payload(&integer_type.pool, cell);
    assert(copy->resource != original.resource && copy->number == 1 && original.address == &original);
    end_insertion(method);
    assert(integer_drops == 1 && original.resource != NULL);
    program.constants = NULL; program.constant_count = 0;
    status = integer_destroy(&original);
    assert(status.code == AMA_OK);
    close_insertion(service);
    finish_insertion();
}

/** Preflight Busy and native Busy share a code but have opposite refusal flags. */
static void it_distinguishes_source_unavailability_from_native_constructor_failure(void) {
    setup_insertion();
    AmaHandle source;
    AmaStatus status = ama_object_create(&world, 1, NULL, &source);
    assert(status.code == AMA_OK);
    owner_value(0)->source = source;
    AmaPoolCell *cell = NULL;
    status = ama_program_cell(&program, source, &cell);
    assert(status.code == AMA_OK);
    cell->metadata.service.activity = AMA_INSTANCE_RUNNING;
    AmaProgramListService *service = open_insertion(1);
    static const AmaProgramAccess access = {.field = 3};
    const AmaProgramInput input = {.source = {.kind = AMA_SOURCE_LOCAL, .access = &access, .type_id = 1}};
    const AmaProgramListInputs supplied = {.inputs = &input, .count = 1, .integer_type = 3};
    AmaProgramListMethod *method = &owner_value(0)->methods[0];
    bool refused;
    status = ama_program_list_append(service, method, &supplied, &refused);
    assert(status.code == AMA_BUSY && refused && service->list->count == 0 && element_clones == 0);
    end_insertion(method);
    cell->metadata.service.activity = AMA_INSTANCE_READY;
    owner_value(0)->source = (AmaHandle){1, 999, 1};
    status = ama_program_list_append(service, method, &supplied, &refused);
    assert(status.code == AMA_STALE_REFERENCE && refused && service->list->count == 0);
    end_insertion(method);
    owner_value(0)->source = source;
    result_type.hooks.clone = insertion_busy_clone;
    status = ama_program_list_append(service, method, &supplied, &refused);
    assert(status.code == AMA_BUSY && !refused && service->list->count == 0 && drops == 0);
    end_insertion(method);
    result_type.hooks.initialize = insertion_busy_constructor;
    const AmaProgramListInputs omitted = {.integer_type = 3};
    status = ama_program_list_append(service, method, &omitted, &refused);
    assert(status.code == AMA_BUSY && !refused && service->list->count == 0 && !method->has_result);
    end_insertion(method);
    close_insertion(service);
    status = ama_object_destroy(&world, source);
    assert(status.code == AMA_OK);
    finish_insertion();
}

/** Visit every allocation in a fresh insertion prefix until the first complete
 * success. Each failed preparation preserves the empty collection and owns no
 * leaked integer, native element or partial membership credit.
 */
static void it_preserves_public_state_after_each_insertion_allocation_failure(void) {
    for (size_t ordinal = 1;; ordinal++) {
        setup_insertion();
        AmaProgramListService *service = open_insertion(1);
        const AmaProgramListInputs plan = {.integer_type = 3};
        AmaProgramListMethod *method = &owner_value(0)->methods[0];
        failure_at = allocation_count + ordinal;
        bool refused;
        AmaStatus status = ama_program_list_insert(service, method, &plan, &refused);
        bool success = status.code == AMA_OK;
        assert(!refused && (success || status.code == AMA_OUT_OF_MEMORY));
        assert(service->list->count == (success ? 1u : 0u));
        failure_at = 0;
        end_insertion(method);
        close_insertion(service);
        finish_insertion();
        if (success) { break; }
    }
}

/** Execute all independently observable typed insertion laws. */
void run(void) {
    it_appends_defaults_and_independent_owned_clones();
    it_appends_complete_reference_handles_without_cloning_targets();
    it_inserts_with_exact_position_conversion_and_preflight_provenance();
    it_copies_supplied_integer_inputs_and_inserts_at_the_end();
    it_distinguishes_source_unavailability_from_native_constructor_failure();
    it_preserves_public_state_after_each_insertion_allocation_failure();
}
/** Return success only after every real pool/native resource is released. */
int main(void) { run(); return EXIT_SUCCESS; }
