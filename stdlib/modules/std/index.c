/**
 * @file
 * @brief Represent retained bounded size_t index payloads with checked decimal conversion.
 *
 * All inputs/views are operation-scoped; generated interfaces establish the native naming context. See the scalar library contract for the explicit standard-root binding.
 * @see [Fixture and coverage contract](../../../docs/implementation/library-tests-and-build.md)
 * [LIB-01](../../../spec/language/library.md#lib-01)
 * binds this native scalar by its explicit standard-root nominal identity.
 */
#include "ama_n_11ed38831778193d6b8d6716738351485dc0bd3813fc41ed3c4a3c5bb54a77d5.interface.h"
#include "ama_n_11ed38831778193d6b8d6716738351485dc0bd3813fc41ed3c4a3c5bb54a77d5.begin.h"

/**
 * Execute this explicit native body synchronously over borrowed parameter/result views. Set
 * status on every return and retain no view; authored effects are defined by the enclosing
 * module fixture.
 */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_d5d6ee4729fede2ad3610308f2cd36990c76dfc25bcf5a9db3e3e370a44c8c8c *parameters, ama_t_70b07b1e0214eb699adcac15b588f83b62081a7694ba263445b1b7f4fa0f5763 *results,
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

#include "ama_n_11ed38831778193d6b8d6716738351485dc0bd3813fc41ed3c4a3c5bb54a77d5.end.h"
