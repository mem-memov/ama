#include "ama_n_5872038ba96f7054949ef1f548e74751fe5e5e1167ceeb923383318a83c085dc.interface.h"
#include "ama_n_5872038ba96f7054949ef1f548e74751fe5e5e1167ceeb923383318a83c085dc.begin.h"

AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_e1b37561440520e4518a451a38b874c265c14d70b04dafd12318aadd3fd48c8a *parameters, ama_t_c45e2a38927349057a787b832fb3397687446949664134245f019255c5012458 *results,
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

#include "ama_n_5872038ba96f7054949ef1f548e74751fe5e5e1167ceeb923383318a83c085dc.end.h"
