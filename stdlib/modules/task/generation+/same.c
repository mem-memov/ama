/** @file
 * @brief Compare two nominal generation snapshots through ordinary typed inputs.
 * [ASYNC-04](../../../../spec/language/async-and-parallelism.md#async-04)
 * keeps every scheduler relationship in declared Ama state and checks scalar publication.
 */
#include "ama_n_3018e533d260e2543feb5d95e79e271d9e55a2766ad322f8690bd26a498e8ef1.interface.h"
#include "ama_n_3018e533d260e2543feb5d95e79e271d9e55a2766ad322f8690bd26a498e8ef1.begin.h"
/** Execute one explicit scalar operation over synchronous typed borrows. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_1a4c29fd3666f0e35d9a7237fd18f9c037c0f888de6430a8b75c8048d698cb55 *parameters, ama_t_ed90a9c5379a3632dda890783bc93d4d7647596116fbe5931a85b5ede42bd638 *results,
    const AmaNativeServices *services, AmaStatus *status) {
    (void)state; (void)parameters; (void)results; (void)services;
    results->AMA_OUTPUT(0)->payload->value = parameters->AMA_INPUT(0)->payload->value
        == parameters->AMA_INPUT(1)->payload->value;
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
#include "ama_n_3018e533d260e2543feb5d95e79e271d9e55a2766ad322f8690bd26a498e8ef1.end.h"
