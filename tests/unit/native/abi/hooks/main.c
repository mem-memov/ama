#include "../common.c"

#include <stdio.h>
#include <string.h>
#include "project/buffer.h"
#include "native/status/index.h"

static void it_checks_all_six_bound_function_signatures(void) {
    AmaStatus (*initialize)(FixtureBuffer *) = fixture_initialize;
    AmaStatus (*destroy)(FixtureBuffer *) = fixture_destroy;
    AmaStatus (*literal)(FixtureBuffer *, AmaByteView) = fixture_from_literal;
    AmaStatus (*clone)(FixtureBuffer *, const FixtureBuffer *) = fixture_clone;
    AmaStatus (*condition)(const FixtureBuffer *, bool *) = fixture_to_condition;
    AmaStatus (*index)(const FixtureBuffer *, size_t *) = fixture_to_index;
    assert(initialize && destroy && literal && clone && condition && index);
}

static void it_constructs_once_and_clones_independent_native_resources(void) {
    FixtureBuffer empty, source, copy;
    AmaStatus status = fixture_initialize(&empty);
    assert(ama_status_is_success(status) && empty.data == NULL);
    const unsigned char bytes[] = {'a', 0, 'b'};
    status = fixture_from_literal(&source, (AmaByteView) {bytes, sizeof(bytes)});
    assert(ama_status_is_success(status));
    status = fixture_clone(&copy, &source);
    assert(ama_status_is_success(status));
    assert(copy.data != source.data && copy.length == 3);
    copy.data[0] = 'z';
    assert(source.data[0] == 'a' && copy.data[2] == 'b');
    status = fixture_destroy(&empty);
    assert(ama_status_is_success(status));
    status = fixture_destroy(&source);
    assert(ama_status_is_success(status));
    status = fixture_destroy(&copy);
    assert(ama_status_is_success(status));
}

static void it_distinguishes_invalid_indices_from_technical_failure_without_outputs(void) {
    FixtureBuffer value;
    const unsigned char negative[] = "-1";
    AmaStatus status = fixture_from_literal(&value, (AmaByteView) {negative, 2});
    assert(ama_status_is_success(status));
    size_t result = 99;
    status = fixture_to_index(&value, &result);
    assert(ama_index_outcome(status) == AMA_INDEX_REFUSED && result == 99);
    value.conversion_error = AMA_OUT_OF_MEMORY;
    status = fixture_to_index(&value, &result);
    assert(ama_index_outcome(status) == AMA_INDEX_FAILED && result == 99);
    bool condition = false;
    status = fixture_to_condition(&value, &condition);
    assert(status.code == AMA_OUT_OF_MEMORY && !condition);
    status = fixture_destroy(&value);
    assert(ama_status_is_success(status));
}

static void it_accepts_size_max_and_refuses_arithmetic_overflow(void) {
    char text[sizeof(size_t) * CHAR_BIT + 2];
    int length = snprintf(text, sizeof(text), "%zu", SIZE_MAX);
    assert(length > 0 && (size_t) length < sizeof(text));
    FixtureBuffer value;
    AmaStatus status = fixture_from_literal(&value,
        (AmaByteView) {(const unsigned char *) text, (size_t) length});
    assert(ama_status_is_success(status));
    size_t result = 0;
    status = fixture_to_index(&value, &result);
    assert(ama_status_is_success(status) && result == SIZE_MAX);
    status = fixture_destroy(&value);
    assert(ama_status_is_success(status));
    memset(text, '9', sizeof(text));
    status = fixture_from_literal(&value,
        (AmaByteView) {(const unsigned char *) text, sizeof(text)});
    assert(ama_status_is_success(status));
    result = 77;
    status = fixture_to_index(&value, &result);
    assert(status.code == AMA_INVALID_INDEX && result == 77);
    status = fixture_destroy(&value);
    assert(ama_status_is_success(status));
}

static void it_returns_declared_results_and_refusal_data_separately_from_technical_status(void) {
    FixtureBuffer state, caller, bound;
    AmaStatus status = fixture_initialize(&state);
    assert(ama_status_is_success(status));
    const unsigned char bytes[] = "12";
    status = fixture_from_literal(&caller, (AmaByteView) {bytes, 2});
    assert(ama_status_is_success(status));
    status = fixture_clone(&bound, &caller);
    assert(ama_status_is_success(status) && bound.data != caller.data);
    FixtureInputs parameters = {&bound};
    FixtureCount length = {0};
    AmaStatus refusal = {0};
    FixtureOutputs results = {&length, &refusal};
    AmaNativeServices services = {ama_status_make(AMA_BUSY, NULL, 0)};
    fixture_entry(&state, &parameters, &results, &services, &status);
    assert(ama_status_is_success(status) && length.value == 2);
    assert(refusal.code == AMA_BUSY && services.caller_refusal.code == AMA_BUSY);
    assert(state.entry_count == 1 && caller.entry_count == 0);
    bool condition = false;
    status = fixture_to_condition(&bound, &condition);
    assert(ama_status_is_success(status) && condition && bound.entry_count == 0);
    status = fixture_destroy(&bound);
    assert(ama_status_is_success(status));
    status = fixture_destroy(&caller);
    assert(ama_status_is_success(status));
    status = fixture_destroy(&state);
    assert(ama_status_is_success(status));
}

void run(void) {
    it_checks_all_six_bound_function_signatures();
    it_constructs_once_and_clones_independent_native_resources();
    it_distinguishes_invalid_indices_from_technical_failure_without_outputs();
    it_accepts_size_max_and_refuses_arithmetic_overflow();
    it_returns_declared_results_and_refusal_data_separately_from_technical_status();
}

int main(void) {
    run();
    return EXIT_SUCCESS;
}
