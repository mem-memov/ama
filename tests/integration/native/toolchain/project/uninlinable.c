/**
 * @file
 * @brief Use setjmp in a forced-inline entry to require a native compilation failure.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../../docs/implementation/library-tests-and-build.md)
 */
#include "types.h"
#include <setjmp.h>

/**
 * Implement the selected native toolchain probe body; the file contract states whether
 * successful inlining or deliberate rejection is expected.
 */
AMA_NATIVE_ENTRY void ama_fixture_native_entry(Payload *state, const Inputs *parameters,
    Outputs *results, const AmaNativeServices *services, AmaStatus *status) {
    jmp_buf environment;
    *results->value = (size_t) setjmp(environment);
    *status = ama_status_success();
    (void) state;
    (void) parameters;
    (void) services;
}

/**
 * Use the setjmp-based native entry so forced-inline compilation must fail before an executable
 * can be accepted.
 */
int main(void) {
    Payload state = {0};
    size_t input = 0, output = 0;
    Inputs parameters = {&input};
    Outputs results = {&output};
    AmaNativeServices services = {0};
    AmaStatus status;
    ama_fixture_native_entry(&state, &parameters, &results, &services, &status);
    return (int) output;
}
