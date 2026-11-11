#include "types.h"
#include <setjmp.h>

AMA_NATIVE_ENTRY void ama_fixture_native_entry(Payload *state, const Inputs *parameters,
    Outputs *results, const AmaNativeServices *services, AmaStatus *status) {
    jmp_buf environment;
    *results->value = (size_t) setjmp(environment);
    *status = ama_status_success();
    (void) state;
    (void) parameters;
    (void) services;
}

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
