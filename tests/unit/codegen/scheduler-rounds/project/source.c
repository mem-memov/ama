/** @file
 * @brief Bind a standalone event hub and its actual native-only worker provider.
 * [ASYNC-05](../../../../../spec/language/async-and-parallelism.md#async-05)
 * keeps task/registration relationships in declared Ama fields, not this resource.
 * The test executable is not unloadable; loadable-provider code leases remain T099.
 */
#include "ama_n_c172a0cd42a72572c711a39f088c0aa0ec11bb0359e09ce2c10fdce7c41d836a.interface.h"
#include "ama_n_c172a0cd42a72572c711a39f088c0aa0ec11bb0359e09ce2c10fdce7c41d836a.begin.h"
#include "native/events/assembly.h"
/** Default owns no hub until explicit construction. */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *state) { *state = (AMA_NATIVE(State)){0}; return ama_status_success(); }
/** Close native admission first; separately owned callbacks/methods keep native storage alive. */
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) { ama_native_event_service_dispose(&state->service); return ama_status_success(); }
/** Generated owned views bind native leases without storing a pointer into another Ama payload. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state, const ama_t_75cbb588ebc22f32cc8635a8dde3a6aa1466621772a11217cc53923077986924 *parameters,
    ama_t_46bbfde89aa402027bcd99fa156f4483c28529c4b5eee7f008a799aabf18893f *results, const AmaNativeServices *services, AmaStatus *status) {
    (void)services;
    ama_native_event_service_dispose(&state->service);
    *status = ama_native_event_service_create(parameters->AMA_INPUT(0)->payload->value, NULL, &state->service);
    if (status->code == AMA_OK) { *status = ama_native_event_service_lease(state->service, &results->AMA_OUTPUT(0)->payload->lease); }
    if (status->code == AMA_OK) { *status = ama_native_event_service_lease(state->service, &results->AMA_OUTPUT(1)->payload->lease); }
    if (status->code == AMA_OK) { *status = ama_native_event_service_lease(state->service, &results->AMA_OUTPUT(2)->payload->lease); }
    if (status->code == AMA_OK) { *status = ama_native_event_service_lease(state->service, &results->AMA_OUTPUT(3)->payload->lease); }
    if (status->code == AMA_OK) { *status = ama_native_event_service_lease(state->service, &results->AMA_OUTPUT(4)->payload->lease); }
    if (status->code == AMA_OK) { *status = ama_native_event_service_lease(state->service, &results->AMA_OUTPUT(5)->payload->lease); }
    if (status->code == AMA_OK) { *status = ama_native_event_service_lease(state->service, &results->AMA_OUTPUT(6)->payload->lease); }
    if (status->code == AMA_OK) { *status = ama_native_event_service_lease(state->service, &results->AMA_OUTPUT(7)->payload->lease); }
}
#include "ama_n_c172a0cd42a72572c711a39f088c0aa0ec11bb0359e09ce2c10fdce7c41d836a.end.h"
