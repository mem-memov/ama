#include "../common.c"
#include "runtime/call.h"

static size_t execute_count;
static bool fail_execution;

static AmaStatus execute_buffer(void* state, void* frame) {
    execute_count++;
    BufferState* persistent = state;
    PairState* prepared = frame;
    persistent->bytes[0] = 'x';
    AmaStatus status = ama_value_replace((AmaValue){&buffer_type, &prepared->second},
        (AmaValue){&buffer_type, &prepared->first});
    if (status != AMA_STATUS_OK) return status;
    return fail_execution ? AMA_STATUS_LITERAL_INVALID : AMA_STATUS_OK;
}

static const AmaCall call = {
    .state_type = &buffer_type,
    .frame_type = &pair_type,
    .parameters = &pair_fields[0],
    .parameter_count = 1,
    .results = &pair_fields[1],
    .result_count = 1,
    .execute = execute_buffer
};

static BufferState state, input, destination;
static PairState frame;

static void setup_call(void) {
    setup_test();
    execute_count = 0;
    fail_execution = false;
    assert(ama_value_initialize((AmaValue){&buffer_type, &state}) == AMA_STATUS_OK);
    assert(ama_value_initialize((AmaValue){&pair_type, &frame}) == AMA_STATUS_OK);
    assert(ama_value_initialize_literal((AmaValue){&buffer_type, &input},
        (const unsigned char*)"input", 5) == AMA_STATUS_OK);
    assert(ama_value_initialize((AmaValue){&buffer_type, &destination}) == AMA_STATUS_OK);
}

static void destroy_call(void) {
    ama_value_destroy((AmaValue){&buffer_type, &destination});
    ama_value_destroy((AmaValue){&buffer_type, &input});
    ama_value_destroy((AmaValue){&pair_type, &frame});
    ama_value_destroy((AmaValue){&buffer_type, &state});
    destroy_test();
}

static AmaStatus invoke(const AmaCallInput* inputs, size_t count) {
    return ama_call_execute(&call, (AmaValue){&buffer_type, &state}, (AmaValue){&pair_type, &frame}, inputs, count);
}

static void it_prepares_independent_inputs_and_retains_results_after_success(void) {
    setup_call();
    AmaCallInput argument = {0, {&buffer_type, &input}};
    assert(invoke(&argument, 1) == AMA_STATUS_OK);
    assert(execute_count == 1 && live_allocations == 5);
    assert(frame.first.length == 5 && frame.second.length == 5);
    assert(frame.first.bytes != input.bytes && frame.second.bytes != input.bytes);
    assert(frame.first.home == &frame.first && frame.second.home == &frame.second);
    input.bytes[0] = 'z';
    assert(frame.second.bytes[0] == 'i' && state.bytes[0] == 'x');
    assert(ama_call_read(&call, (AmaValue){&pair_type, &frame}, 0,
        (AmaValue){&buffer_type, &destination}) == AMA_STATUS_OK);
    assert(destination.bytes[0] == 'i' && execute_count == 1);
    destroy_call();
}

static void it_reads_initial_defaults_and_resets_omitted_parameters_on_each_execution(void) {
    setup_call();
    assert(ama_call_read(&call, (AmaValue){&pair_type, &frame}, 0,
        (AmaValue){&buffer_type, &destination}) == AMA_STATUS_OK);
    assert(execute_count == 0 && destination.length == 7);
    AmaCallInput argument = {0, {&buffer_type, &input}};
    assert(invoke(&argument, 1) == AMA_STATUS_OK && frame.second.length == 5);
    assert(invoke(NULL, 0) == AMA_STATUS_OK && frame.second.length == 7 && execute_count == 2);
    destroy_call();
}

static void it_discards_failed_native_results_but_does_not_undo_persistent_state_changes(void) {
    setup_call();
    unsigned char* previous = frame.second.bytes;
    fail_execution = true;
    assert(invoke(NULL, 0) == AMA_STATUS_LITERAL_INVALID);
    assert(frame.second.bytes == previous && frame.second.length == 7);
    assert(state.bytes[0] == 'x' && execute_count == 1 && live_allocations == 5);
    fail_execution = false;
    assert(invoke(NULL, 0) == AMA_STATUS_OK && execute_count == 2);
    destroy_call();
}

static void it_does_not_execute_when_default_initialization_or_input_copy_fails(void) {
    setup_call();
    unsigned char* previous = frame.first.bytes;
    fail_initialize_at = initialize_attempts + 2;
    assert(invoke(NULL, 0) == AMA_STATUS_MEMORY);
    assert(live_allocations == 5 && execute_count == 0 && frame.first.bytes == previous);
    fail_initialize_at = 0;
    AmaCallInput argument = {0, {&buffer_type, &input}};
    fail_copy_at = copy_attempts + 1;
    assert(invoke(&argument, 1) == AMA_STATUS_MEMORY);
    assert(live_allocations == 5 && execute_count == 0 && frame.first.bytes == previous);
    destroy_call();
}

static void it_preserves_read_destinations_on_clone_failure(void) {
    setup_call();
    unsigned char* previous = destination.bytes;
    fail_copy_at = copy_attempts + 1;
    assert(ama_call_read(&call, (AmaValue){&pair_type, &frame}, 0,
        (AmaValue){&buffer_type, &destination}) == AMA_STATUS_MEMORY);
    assert(destination.bytes == previous && execute_count == 0 && live_allocations == 5);
    assert(ama_call_read(&call, (AmaValue){&pair_type, &frame}, 1,
        (AmaValue){&buffer_type, &destination}) == AMA_STATUS_INVALID);
    assert(ama_call_read(&call, (AmaValue){&pair_type, &frame}, 0,
        (AmaValue){&buffer_type, &frame.second}) == AMA_STATUS_INVALID);
    destroy_call();
}

static void it_checks_argument_positions_duplicates_and_nominal_types_before_preparation(void) {
    setup_call();
    size_t previous = initialize_attempts;
    AmaCallInput arguments[] = {{0, {&buffer_type, &input}}, {0, {&buffer_type, &input}}};
    assert(invoke(arguments, 2) == AMA_STATUS_INVALID);
    arguments[0].position = 1;
    assert(invoke(arguments, 1) == AMA_STATUS_INVALID);
    arguments[0].position = 0;
    arguments[0].value.type = &number_type;
    assert(invoke(arguments, 1) == AMA_STATUS_TYPE_MISMATCH);
    assert(invoke(NULL, 1) == AMA_STATUS_INVALID);
    assert(initialize_attempts == previous && execute_count == 0);
    destroy_call();
}

static void it_rejects_invalid_call_metadata_and_overlapping_instance_storage(void) {
    setup_call();
    AmaCall altered = call;
    altered.results = altered.parameters;
    assert(ama_call_execute(&altered, (AmaValue){&buffer_type, &state},
        (AmaValue){&pair_type, &frame}, NULL, 0) == AMA_STATUS_INVALID);
    altered = call;
    altered.execute = NULL;
    assert(ama_call_execute(&altered, (AmaValue){&buffer_type, &state},
        (AmaValue){&pair_type, &frame}, NULL, 0) == AMA_STATUS_INVALID);
    assert(ama_call_execute(&call, (AmaValue){&buffer_type, &frame.first},
        (AmaValue){&pair_type, &frame}, NULL, 0) == AMA_STATUS_INVALID);
    assert(execute_count == 0);
    destroy_call();
}

static void it_snapshots_inputs_from_the_previously_retained_frame(void) {
    setup_call();
    AmaCallInput argument = {0, {&buffer_type, &frame.second}};
    assert(invoke(&argument, 1) == AMA_STATUS_OK && frame.second.length == 7);
    assert(frame.first.home == &frame.first && live_allocations == 5);
    destroy_call();
}

static void run(void) {
    it_prepares_independent_inputs_and_retains_results_after_success();
    it_reads_initial_defaults_and_resets_omitted_parameters_on_each_execution();
    it_discards_failed_native_results_but_does_not_undo_persistent_state_changes();
    it_does_not_execute_when_default_initialization_or_input_copy_fails();
    it_preserves_read_destinations_on_clone_failure();
    it_checks_argument_positions_duplicates_and_nominal_types_before_preparation();
    it_rejects_invalid_call_metadata_and_overlapping_instance_storage();
    it_snapshots_inputs_from_the_previously_retained_frame();
}

int main(void) {
    run();
    printf("unit/runtime/calls: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
