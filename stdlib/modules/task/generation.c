/** @file
 * @brief Advance a declared generation scalar only after reserving a nonwrapping successor.
 * [ASYNC-04](../../../spec/language/async-and-parallelism.md#async-04)
 * keeps every scheduler relationship in declared Ama state and checks scalar publication.
 */
#include "ama_n_a7fa5ca88d01ea11806a55d1390a8bc45170135ae628f5b00b3cbe00d626ca33.interface.h"
#include "ama_n_a7fa5ca88d01ea11806a55d1390a8bc45170135ae628f5b00b3cbe00d626ca33.begin.h"
#include "generation+/native/advance.h"
/** Execute one explicit scalar operation over synchronous typed borrows. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_92c9b7db521de1223828a1d2f47159c1e63a09333abd3792fa447b6b911a8a27 *parameters, ama_t_c15fe5ca9d70e8a08dcdbc4a115e762360a139942a52483791e7d7ea5cf88cf9 *results,
    const AmaNativeServices *services, AmaStatus *status) {
    (void)state; (void)parameters; (void)results; (void)services;
    size_t next;
    if (!AMA_NATIVE(reserve)(state->value, SIZE_MAX, &next)) {
        *status = (AmaStatus){AMA_NATIVE_FAILURE, "Task generation exhausted", 25};
        return;
    }
    state->value = next;
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
/** Replace only a fully parsed scalar; failed literal conversion preserves the prior state. */
AmaStatus AMA_NATIVE(literal)(AMA_NATIVE(State) *destination, AmaByteView text) {
    size_t value;
    if (!AMA_NATIVE(parse)(text, &value)) {
        return (AmaStatus){AMA_INVALID_LITERAL, "Invalid task generation", 23};
    }
    destination->value = value;
    return ama_status_success();
}
/** Read whether this scalar has advanced beyond its reserved zero value. */
AmaStatus AMA_NATIVE(condition)(const AMA_NATIVE(State) *state, bool *result) {
    *result = state->value != 0;
    return ama_status_success();
}
#include "ama_n_a7fa5ca88d01ea11806a55d1390a8bc45170135ae628f5b00b3cbe00d626ca33.end.h"
