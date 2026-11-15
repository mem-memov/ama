/** @file
 * @brief Commit one cooperative terminal outcome and republish its ordinary predicate results.
 * [ASYNC-08](../../../spec/language/async-and-parallelism.md#async-08)
 * forbids later cancellation or completion from replacing the first committed result.
 */
#include "ama_n_6f0e2bd05f141bdae43d02d53a4590cfaf6b53fa69f625ebed0123f63832778c.interface.h"
#include "ama_n_6f0e2bd05f141bdae43d02d53a4590cfaf6b53fa69f625ebed0123f63832778c.begin.h"
#include "outcome+/native/selection.h"
/** Default construction owns no external resource. */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *state) { *state = (AMA_NATIVE(State)){0}; return ama_status_success(); }
/** Generated Ama fields, not this payload hook, own declared data. */
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) { (void)state; return ama_status_success(); }
/** Copy only the resource-free native value; no subscription or scheduler enrollment is acquired. */
AmaStatus AMA_NATIVE(clone)(AMA_NATIVE(State) *destination, const AMA_NATIVE(State) *source) { *destination = *source; return ama_status_success(); }

/** The terminal-state assignment is the linearization point; the single executor observes all predicates together on return. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state, const ama_t_22803ba5f0a2283aad97e5311f82953facd8e1a1d36a2cdf05e0b7021b2d0ad1 *parameters,
    ama_t_d2f0b9951ddacd35a2f29b924e7d4d5562066d5dc6e54dfa4db3131766ab25d7 *results, const AmaNativeServices *services, AmaStatus *status) {
    (void)services;
    unsigned selected;
    if (!AMA_NATIVE(select)(state->terminal, parameters->AMA_INPUT(0)->payload->value,
            parameters->AMA_INPUT(1)->payload->value, &selected)) {
        *status = ama_status_make(AMA_INVALID_STATE, NULL, 0); return;
    }
    state->terminal = selected;
    results->AMA_OUTPUT(0)->payload->value = selected == 0;
    results->AMA_OUTPUT(1)->payload->value = selected == 1;
    results->AMA_OUTPUT(2)->payload->value = selected == 2;
    *status = ama_status_success();
}
#include "ama_n_6f0e2bd05f141bdae43d02d53a4590cfaf6b53fa69f625ebed0123f63832778c.end.h"
