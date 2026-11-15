/** @file
 * @brief Bind a standalone event hub and its actual native-only worker provider.
 * [ASYNC-05](../../../../../spec/language/async-and-parallelism.md#async-05)
 * keeps task/registration relationships in declared Ama fields, not this resource.
 * The test executable is not unloadable; loadable-provider code leases remain T099.
 */
#include "ama_n_149726ccf294647d6e5cd6d5a3cb6f614963b43c5c62c9c0460b3b12d35788bc.interface.h"
#include "ama_n_149726ccf294647d6e5cd6d5a3cb6f614963b43c5c62c9c0460b3b12d35788bc.begin.h"
#include "native/events/assembly.h"
/** Default owns no hub until explicit construction. */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *state) { *state = (AMA_NATIVE(State)){0}; return ama_status_success(); }
/** Close native admission first; separately owned callbacks/methods keep native storage alive. */
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) { ama_native_event_service_dispose(&state->service); return ama_status_success(); }
/** Generated owned views bind native leases without storing a pointer into another Ama payload. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state, const ama_t_cf3e102bc72f22d7853b8f21602714a8d51fc5c0c70abaa5da4e0c1f0109f0dc *parameters,
    ama_t_297175714a35a317a47d87cc6f5888f9bfb642091438aa85f20541352a67d575 *results, const AmaNativeServices *services, AmaStatus *status) {
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
#include "ama_n_149726ccf294647d6e5cd6d5a3cb6f614963b43c5c62c9c0460b3b12d35788bc.end.h"
