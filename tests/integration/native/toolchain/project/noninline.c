#include "types.h"

__attribute__((noinline)) void ama_fixture_native_entry(Payload *state, const Inputs *parameters,
    Outputs *results, const AmaNativeServices *services, AmaStatus *status) {
    *results->value = *parameters->value;
    *status = ama_status_success();
    (void) state;
    (void) services;
}

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
