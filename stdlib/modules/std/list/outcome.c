#include "ama_n_e78ab229ad3492496caf4ec0f9febb6a8b49b58439d1716111d1830d10d101d0.interface.h"
#include "ama_n_e78ab229ad3492496caf4ec0f9febb6a8b49b58439d1716111d1830d10d101d0.begin.h"

AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_9fb5f9300bf149ea6f04fad0f798d75a319eeb83d267de23b8976cb066209a91 *parameters, ama_t_3061d689367f451d206c22c075225be90c40e10d77857a762d1a01573d3542d1 *results,
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
    if (number != 0 && number != 7 && number != 8 && number != 9 && number != 12) { return ama_status_make(AMA_INVALID_LITERAL, NULL, 0); }
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

#include "ama_n_e78ab229ad3492496caf4ec0f9febb6a8b49b58439d1716111d1830d10d101d0.end.h"
