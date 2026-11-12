/**
 * @file
 * @brief Demonstrate inline native entry, permitted ordinary C recursion and label-based caller return.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../../docs/implementation/library-tests-and-build.md)
 */
#include "types.h"
#include <stdlib.h>

/**
 * Demonstrate ordinary recursive C helper calls remain permitted inside an inlined native entry;
 * this is not recursive Ama dispatch.
 */
__attribute__((noinline, noipa)) size_t allowed_helper(size_t value) {
    return value == 0 ? 0 : value + allowed_helper(value - 1);
}

/**
 * Implement the selected native toolchain probe body; the file contract states whether
 * successful inlining or deliberate rejection is expected.
 */
AMA_NATIVE_ENTRY void ama_fixture_native_entry(Payload *state, const Inputs *parameters,
    Outputs *results, const AmaNativeServices *services, AmaStatus *status) {
    state->total += allowed_helper(*parameters->value);
    *results->value = state->total;
    *status = ama_status_success();
    (void) services;
}

/** Finite probe instance storage with explicit busy/caller/return metadata; this handwritten acceptance model is not the production execution-context implementation. */
typedef struct {
    Payload payload;
    size_t input, output, caller, return_selector;
    bool busy;
} Instance;

static struct {
    Instance *instances;
    size_t current, selector;
    AmaStatus status;
} execution;

/**
 * Run the finite handwritten label-dispatch probe, invoke the inline native entry and verify
 * caller output/busy state before releasing its instance array.
 */
int main(void) {
    execution.instances = calloc(2, sizeof(Instance));
    if (execution.instances == NULL) {
        return EXIT_FAILURE;
    }
    execution.current = 0;
    goto module_a;
module_a:
    execution.instances[0].busy = true;
    execution.instances[1].caller = 0;
    execution.instances[1].return_selector = 1;
    execution.instances[1].input = 3;
    execution.current = 1;
    goto module_b;
module_a_return:
    if (!execution.instances[0].busy || execution.instances[1].busy ||
        execution.instances[0].output != 6) {
        goto failed;
    }
    free(execution.instances);
    return EXIT_SUCCESS;
module_b: {
    Instance *instance = &execution.instances[execution.current];
    instance->busy = true;
    Inputs parameters = {&instance->input};
    Outputs results = {&instance->output};
    AmaNativeServices services = {0};
    ama_fixture_native_entry(&instance->payload, &parameters, &results,
        &services, &execution.status);
    if (!ama_status_is_success(execution.status)) {
        goto failed;
    }
    execution.selector = instance->return_selector;
    execution.current = instance->caller;
    execution.instances[execution.current].output = instance->output;
    instance->busy = false;
}
    switch (execution.selector) {
        case 1: goto module_a_return;
        default: goto failed;
    }
failed:
    free(execution.instances);
    return EXIT_FAILURE;
}
