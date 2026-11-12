/**
 * @file
 * @brief Retain an ordinary C entry symbol so verification must reject its object.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../../docs/implementation/library-tests-and-build.md)
 */
#include "types.h"

/**
 * Deliberately retain an externally visible ordinary C native-entry function; acceptance must
 * reject its surviving exact symbol before linking/publishing.
 */
__attribute__((noinline)) void ama_fixture_native_entry(Payload *state, const Inputs *parameters,
    Outputs *results, const AmaNativeServices *services, AmaStatus *status) {
    *results->value = *parameters->value;
    *status = ama_status_success();
    (void) state;
    (void) services;
}

/**
 * Call the deliberately noninlined native entry so its object retains a symbol that production
 * verification must reject.
 */
int main(void) {
    Payload state = {0};
    size_t input = 3, output = 0;
    Inputs parameters = {&input};
    Outputs results = {&output};
    AmaNativeServices services = {0};
    AmaStatus status;
    ama_fixture_native_entry(&state, &parameters, &results, &services, &status);
    return output == input ? 0 : 1;
}
