/** @file
 * @brief Compare two nominal token snapshots through ordinary typed inputs.
 * [ASYNC-04](../../../../spec/language/async-and-parallelism.md#async-04)
 * keeps every scheduler relationship in declared Ama state and checks scalar publication.
 */
#include "ama_n_48c3872237b6836476a4dfce54f302ebb908bf3692bbcc8a3ad2fee2a30ee351.interface.h"
#include "ama_n_48c3872237b6836476a4dfce54f302ebb908bf3692bbcc8a3ad2fee2a30ee351.begin.h"
/** Execute one explicit scalar operation over synchronous typed borrows. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_449025c9ccfd60424fdabcf0ebaa835a1537f48bd25624130269a692a6c96d8c *parameters, ama_t_e90a3e924979f1c07c4ecea07dc826d511a225982b0bf72541e2cdbc7da37be5 *results,
    const AmaNativeServices *services, AmaStatus *status) {
    (void)state; (void)parameters; (void)results; (void)services;
    results->AMA_OUTPUT(0)->payload->value = ama_native_event_token_equal(parameters->AMA_INPUT(0)->payload->token, parameters->AMA_INPUT(1)->payload->token);
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
#include "ama_n_48c3872237b6836476a4dfce54f302ebb908bf3692bbcc8a3ad2fee2a30ee351.end.h"
