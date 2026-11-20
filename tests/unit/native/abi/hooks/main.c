/**
 * @file
 * @brief Exercise all native lifecycle/conversion hooks with owned byte buffers and refusal snapshots.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../../docs/implementation/library-tests-and-build.md)
 */
#include "../common.c"

#include <stdio.h>
#include <string.h>
// Fixed NAME-05 context for the fixture identity /test/fixture/v1.0.0/src/fixtures/buffer.
#define AMA_NATIVE(local_name) ama_n_8be7d506080a13c6aacf712f924b5bb774a8782a5c786540d9d4e49ad7ed49ae_##local_name
#include "project/buffer.c"
#include "native/status/index.h"

/**
 * Assert that the component checks all six bound function signatures. The explicit fixture setup
 * and observations define this regression; assertion failure keeps its generated evidence.
 */
static void it_checks_all_six_bound_function_signatures(void) {
    AmaStatus (*initialize)(AMA_NATIVE(FixtureBuffer) *) = AMA_NATIVE(initialize);
    AmaStatus (*destroy)(AMA_NATIVE(FixtureBuffer) *) = AMA_NATIVE(destroy);
    AmaStatus (*literal)(AMA_NATIVE(FixtureBuffer) *, AmaByteView) = AMA_NATIVE(from_literal);
    AmaStatus (*clone)(AMA_NATIVE(FixtureBuffer) *, const AMA_NATIVE(FixtureBuffer) *) = AMA_NATIVE(clone);
    AmaStatus (*condition)(const AMA_NATIVE(FixtureBuffer) *, bool *) = AMA_NATIVE(to_condition);
    AmaStatus (*index)(const AMA_NATIVE(FixtureBuffer) *, size_t *) = AMA_NATIVE(to_index);
    assert(initialize && destroy && literal && clone && condition && index);
}

/**
 * Assert that the component constructs once and clones independent native resources. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_constructs_once_and_clones_independent_native_resources(void) {
    AMA_NATIVE(FixtureBuffer) empty, source, copy;
    AmaStatus status = AMA_NATIVE(initialize)(&empty);
    assert(ama_status_is_success(status) && empty.data == NULL);
    const unsigned char bytes[] = {'a', 0, 'b'};
    status = AMA_NATIVE(from_literal)(&source, (AmaByteView) {bytes, sizeof(bytes)});
    assert(ama_status_is_success(status));
    status = AMA_NATIVE(clone)(&copy, &source);
    assert(ama_status_is_success(status));
    assert(copy.data != source.data && copy.length == 3);
    copy.data[0] = 'z';
    assert(source.data[0] == 'a' && copy.data[2] == 'b');
    status = AMA_NATIVE(destroy)(&empty);
    assert(ama_status_is_success(status));
    status = AMA_NATIVE(destroy)(&source);
    assert(ama_status_is_success(status));
    status = AMA_NATIVE(destroy)(&copy);
    assert(ama_status_is_success(status));
}

/**
 * Assert that the component distinguishes invalid indices from technical failure without
 * outputs. The explicit fixture setup and observations define this regression; assertion failure
 * keeps its generated evidence.
 */
static void it_distinguishes_invalid_indices_from_technical_failure_without_outputs(void) {
    AMA_NATIVE(FixtureBuffer) value;
    const unsigned char negative[] = "-1";
    AmaStatus status = AMA_NATIVE(from_literal)(&value, (AmaByteView) {negative, 2});
    assert(ama_status_is_success(status));
    size_t result = 99;
    status = AMA_NATIVE(to_index)(&value, &result);
    assert(ama_index_outcome(status) == AMA_INDEX_REFUSED && result == 99);
    value.conversion_error = AMA_OUT_OF_MEMORY;
    status = AMA_NATIVE(to_index)(&value, &result);
    assert(ama_index_outcome(status) == AMA_INDEX_FAILED && result == 99);
    bool condition = false;
    status = AMA_NATIVE(to_condition)(&value, &condition);
    assert(status.code == AMA_OUT_OF_MEMORY && !condition);
    status = AMA_NATIVE(destroy)(&value);
    assert(ama_status_is_success(status));
}

/**
 * Assert that the component accepts size max and refuses arithmetic overflow. The explicit
 * fixture setup and observations define this regression; assertion failure keeps its generated
 * evidence.
 */
static void it_accepts_size_max_and_refuses_arithmetic_overflow(void) {
    char text[sizeof(size_t) * CHAR_BIT + 2];
    int length = snprintf(text, sizeof(text), "%zu", SIZE_MAX);
    assert(length > 0 && (size_t) length < sizeof(text));
    AMA_NATIVE(FixtureBuffer) value;
    AmaStatus status = AMA_NATIVE(from_literal)(&value,
        (AmaByteView) {(const unsigned char *) text, (size_t) length});
    assert(ama_status_is_success(status));
    size_t result = 0;
    status = AMA_NATIVE(to_index)(&value, &result);
    assert(ama_status_is_success(status) && result == SIZE_MAX);
    status = AMA_NATIVE(destroy)(&value);
    assert(ama_status_is_success(status));
    memset(text, '9', sizeof(text));
    status = AMA_NATIVE(from_literal)(&value,
        (AmaByteView) {(const unsigned char *) text, sizeof(text)});
    assert(ama_status_is_success(status));
    result = 77;
    status = AMA_NATIVE(to_index)(&value, &result);
    assert(status.code == AMA_INVALID_INDEX && result == 77);
    status = AMA_NATIVE(destroy)(&value);
    assert(ama_status_is_success(status));
}

/**
 * Assert that the component returns declared results and refusal data separately from technical
 * status. The explicit fixture setup and observations define this regression; assertion failure
 * keeps its generated evidence.
 */
static void it_returns_declared_results_and_refusal_data_separately_from_technical_status(void) {
    AMA_NATIVE(FixtureBuffer) state, caller, bound;
    AmaStatus status = AMA_NATIVE(initialize)(&state);
    assert(ama_status_is_success(status));
    const unsigned char bytes[] = "12";
    status = AMA_NATIVE(from_literal)(&caller, (AmaByteView) {bytes, 2});
    assert(ama_status_is_success(status));
    status = AMA_NATIVE(clone)(&bound, &caller);
    assert(ama_status_is_success(status) && bound.data != caller.data);
    AMA_NATIVE(FixtureInputs) parameters = {&bound};
    AMA_NATIVE(FixtureCount) length = {0};
    AmaStatus refusal = {0};
    AMA_NATIVE(FixtureOutputs) results = {&length, &refusal};
    AmaNativeServices services = {.caller_refusal = ama_status_make(AMA_BUSY, NULL, 0)};
    AMA_NATIVE(entry)(&state, &parameters, &results, &services, &status);
    assert(ama_status_is_success(status) && length.value == 2);
    assert(refusal.code == AMA_BUSY && services.caller_refusal.code == AMA_BUSY);
    assert(services.code.resource == NULL && services.code.retain == NULL);
    assert(state.entry_count == 1 && caller.entry_count == 0);
    bool condition = false;
    status = AMA_NATIVE(to_condition)(&bound, &condition);
    assert(ama_status_is_success(status) && condition && bound.entry_count == 0);
    status = AMA_NATIVE(destroy)(&bound);
    assert(ama_status_is_success(status));
    status = AMA_NATIVE(destroy)(&caller);
    assert(ama_status_is_success(status));
    status = AMA_NATIVE(destroy)(&state);
    assert(ama_status_is_success(status));
}

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_checks_all_six_bound_function_signatures();
    it_constructs_once_and_clones_independent_native_resources();
    it_distinguishes_invalid_indices_from_technical_failure_without_outputs();
    it_accepts_size_max_and_refuses_arithmetic_overflow();
    it_returns_declared_results_and_refusal_data_separately_from_technical_status();
}

/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}

#undef AMA_NATIVE
