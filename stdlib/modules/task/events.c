/** @file
 * @brief Construct a native queue and bind its actual selected methods without hidden Ama references.
 * [ASYNC-05](../../../spec/language/async-and-parallelism.md#async-05)
 * retains native completions independently of executor-owned registration state.
 */
#include "ama_n_f97a1197920dc93c3ab22e9454f4d908be2f20f308025795aed207c8c5f8fd28.interface.h"
#include "ama_n_f97a1197920dc93c3ab22e9454f4d908be2f20f308025795aed207c8c5f8fd28.begin.h"
#include "native/events/assembly.h"
/** Default service owns no native queue until explicit construction. */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *state) { *state = (AMA_NATIVE(State)){0}; return ama_status_success(); }
/** Close before releasing owner rights; retained method/callback/byte owners retire independently. */
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) { ama_native_event_service_dispose(&state->service); return ama_status_success(); }
/** Reinitialization explicitly retires the old service; generated entry resets method results first. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state, const ama_t_b1cd85f20ecffcce31e9c6ddf93db2f031204ea4a6c8cd4c67199370d52dc1a4 *parameters,
    ama_t_840e5ddcfe54322779ee463abc8e9a290908495fee453015a7b9f907908b7bcf *results, const AmaNativeServices *services, AmaStatus *status) {
    (void)services;
    ama_native_event_service_dispose(&state->service);
    *status = ama_native_event_service_create(parameters->AMA_INPUT(0)->payload->value, NULL, &state->service);
    if (status->code == AMA_OK) { *status = ama_native_event_service_lease(state->service, &results->AMA_OUTPUT(0)->payload->lease); }
    if (status->code == AMA_OK) { *status = ama_native_event_service_lease(state->service, &results->AMA_OUTPUT(1)->payload->lease); }
    if (status->code == AMA_OK) { *status = ama_native_event_service_lease(state->service, &results->AMA_OUTPUT(2)->payload->lease); }
    if (status->code == AMA_OK) { *status = ama_native_event_service_lease(state->service, &results->AMA_OUTPUT(3)->payload->lease); }
    if (status->code == AMA_OK) { *status = ama_native_event_service_lease(state->service, &results->AMA_OUTPUT(4)->payload->lease); }
}
#include "ama_n_f97a1197920dc93c3ab22e9454f4d908be2f20f308025795aed207c8c5f8fd28.end.h"
