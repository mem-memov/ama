/**
 * @file
 * @brief Represent retained binary condition payloads with checked decimal literal conversion.
 *
 * All inputs/views are operation-scoped; generated interfaces establish the native naming context. See the retained library contract and pending scalar roadmap.
 * @see [Fixture and coverage contract](../../../docs/implementation/library-tests-and-build.md)
 */
#include "ama_n_32e46f3e4df34bff062fc3fc87737294ebfaa344168ad7cac0eaac152a38913a.interface.h"
#include "ama_n_32e46f3e4df34bff062fc3fc87737294ebfaa344168ad7cac0eaac152a38913a.begin.h"

/**
 * Execute this explicit native body synchronously over borrowed parameter/result views. Set
 * status on every return and retain no view; authored effects are defined by the enclosing
 * module fixture.
 */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_319ee73a92bd570a21eb8d47c7e7072f1c47e1c7649b06379f49fc9c91a20562 *parameters, ama_t_b2639cd4f6be1753732ae383f5b533814e78b2dd2fb50eabb0b71e8bf4daf12a *results,
    const AmaNativeServices *services, AmaStatus *status) {
    (void)state; (void)parameters; (void)results; (void)services;
    *status = ama_status_success();
}

/**
 * Initialize one fresh native State with its success default; no prior live resource may occupy
 * destination.
 */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *destination) {
    *destination = (AMA_NATIVE(State)){0};
    return ama_status_success();
}
/**
 * Release only successfully constructed native State. Retained scalar modules own no resource;
 * fixture-specific injected failures are returned after cleanup.
 */
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) {
    (void)state;
    return ama_status_success();
}
/**
 * Copy source native data into fresh destination storage without copying runtime identity or
 * transient execution links; source stays unchanged.
 */
AmaStatus AMA_NATIVE(clone)(AMA_NATIVE(State) *destination, const AMA_NATIVE(State) *source) {
    *destination = *source;
    return ama_status_success();
}
/**
 * Decode bounded literal bytes with checked arithmetic before installing State. Reject
 * malformed/range values and never depend on a terminator.
 */
AmaStatus AMA_NATIVE(literal)(AMA_NATIVE(State) *destination, AmaByteView literal) {
    if (literal.length == 0) { return ama_status_make(AMA_INVALID_LITERAL, NULL, 0); }
    size_t number = 0;
    for (size_t index = 0; index < literal.length; index++) {
        unsigned char digit = literal.data[index];
        if (digit < '0' || digit > '9') { return ama_status_make(AMA_INVALID_LITERAL, NULL, 0); }
        if (number > (SIZE_MAX - (digit - '0')) / 10) { return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0); }
        number = number * 10 + digit - '0';
    }
    if (number > 1) { return ama_status_make(AMA_INVALID_LITERAL, NULL, 0); }
    destination->value = number;
    return ama_status_success();
}
/**
 * Read native payload and publish its explicit condition result without mutating State or
 * retaining the output pointer.
 */
AmaStatus AMA_NATIVE(condition)(const AMA_NATIVE(State) *state, bool *result) {
    *result = state->value != 0;
    return ama_status_success();
}
/**
 * Read the retained exact nonnegative host-size value through the explicit index protocol; no
 * compiler assumption about payload layout is implied.
 */
AmaStatus AMA_NATIVE(index)(const AMA_NATIVE(State) *state, size_t *result) {
    *result = state->value;
    return ama_status_success();
}

#include "ama_n_32e46f3e4df34bff062fc3fc87737294ebfaa344168ad7cac0eaac152a38913a.end.h"
