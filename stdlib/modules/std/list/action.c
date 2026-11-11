#include "ama_n_b8912a4e05f8659855654a6e67cb85eb7ec2a12012aae31cf4ea865343fbbcf6.interface.h"
#include "ama_n_b8912a4e05f8659855654a6e67cb85eb7ec2a12012aae31cf4ea865343fbbcf6.begin.h"

AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_bc7b2cbee955e03907b2faa6925463efb6bc2d6437cc3fb55ddce416cbb5901e *parameters, ama_t_4bbfe7ca7435e9936d13f076e29b12f341632c879bbc7d22192233ab0f6d2ed8 *results,
    const AmaNativeServices *services, AmaStatus *status) {
    (void)state; (void)parameters; (void)results; (void)services;
    *status = ama_status_success();
}

AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *destination) {
    *destination = (AMA_NATIVE(State)){0};
    return ama_status_success();
}
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) {
    (void)state;
    return ama_status_success();
}
AmaStatus AMA_NATIVE(clone)(AMA_NATIVE(State) *destination, const AMA_NATIVE(State) *source) {
    *destination = *source;
    return ama_status_success();
}
AmaStatus AMA_NATIVE(literal)(AMA_NATIVE(State) *destination, AmaByteView literal) {
    if (literal.length == 0) { return ama_status_make(AMA_INVALID_LITERAL, NULL, 0); }
    size_t number = 0;
    for (size_t index = 0; index < literal.length; index++) {
        unsigned char digit = literal.data[index];
        if (digit < '0' || digit > '9') { return ama_status_make(AMA_INVALID_LITERAL, NULL, 0); }
        if (number > (SIZE_MAX - (digit - '0')) / 10) { return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0); }
        number = number * 10 + digit - '0';
    }
    if (number > 5) { return ama_status_make(AMA_INVALID_LITERAL, NULL, 0); }
    destination->value = number;
    return ama_status_success();
}
AmaStatus AMA_NATIVE(condition)(const AMA_NATIVE(State) *state, bool *result) {
    *result = state->value != 0;
    return ama_status_success();
}
AmaStatus AMA_NATIVE(index)(const AMA_NATIVE(State) *state, size_t *result) {
    *result = state->value;
    return ama_status_success();
}

#include "ama_n_b8912a4e05f8659855654a6e67cb85eb7ec2a12012aae31cf4ea865343fbbcf6.end.h"
