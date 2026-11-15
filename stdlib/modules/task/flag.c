/** @file
 * @brief Publish an explicitly supplied scheduler flag through an ordinary typed call.
 * [ASYNC-04](../../../spec/language/async-and-parallelism.md#async-04)
 * keeps every scheduler relationship in declared Ama state and checks scalar publication.
 */
#include "ama_n_d34e0f35a15e21455472011c08e8c45ba351b2bc1f1b6b3906739eab58969d66.interface.h"
#include "ama_n_d34e0f35a15e21455472011c08e8c45ba351b2bc1f1b6b3906739eab58969d66.begin.h"
/** Execute one explicit scalar operation over synchronous typed borrows. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_d68d4f47041f608bd7e60d5f11d79471dec88fc3e643f6d59969147de486a60c *parameters, ama_t_f0eb8d43290796c5e8f3fa1ec7cf3f451fae9dfcc8090c3767a63675457559a6 *results,
    const AmaNativeServices *services, AmaStatus *status) {
    (void)state; (void)parameters; (void)results; (void)services;
    size_t value = parameters->AMA_INPUT(0)->payload->value;
    if (value > 1) {
        *status = (AmaStatus){AMA_NATIVE_FAILURE, "Invalid task flag", 17};
        return;
    }
    state->value = value != 0;
    *status = ama_status_success();
}
/** Initialize a fresh native scalar or empty payload before publication. */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *destination) {
    *destination = (AMA_NATIVE(State)){0};
    return ama_status_success();
}
/** Retire no native resources; declared fields follow generated ownership cleanup. */
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) {
    (void)state;
    return ama_status_success();
}
/** Copy the scalar value without copying any Ama identity or scheduler relationship. */
AmaStatus AMA_NATIVE(clone)(AMA_NATIVE(State) *destination, const AMA_NATIVE(State) *source) {
    *destination = *source;
    return ama_status_success();
}
/** Read membership without executing a setter or retaining the output pointer. */
AmaStatus AMA_NATIVE(condition)(const AMA_NATIVE(State) *state, bool *result) {
    *result = state->value;
    return ama_status_success();
}
#include "ama_n_d34e0f35a15e21455472011c08e8c45ba351b2bc1f1b6b3906739eab58969d66.end.h"
