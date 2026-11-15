/** @file
 * @brief Execute the explicit take operation on a retained native service.
 * [ASYNC-05](../../../../spec/language/async-and-parallelism.md#async-05)
 * allows native records only; Ama state is accessed solely through these generated borrows.
 */
#include "ama_n_be2e35e01b9d693789bf50198841b52f867e802f5dc7d6dedf45b323afe7235d.interface.h"
#include "ama_n_be2e35e01b9d693789bf50198841b52f867e802f5dc7d6dedf45b323afe7235d.begin.h"
#include "native/events/assembly.h"
/** Construct an initially unbound method; the owning constructor binds a native lease. */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *state) { *state = (AMA_NATIVE(State)){0}; return ama_status_success(); }
/** Retire method resource rights without executing Ama or releasing another method's lease. */
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) {
    AmaNativeEventKeepalive lease = state->lease;
    state->lease = (AmaNativeEventKeepalive){0};
    if (lease.release != NULL) { lease.release(lease.resource); }
    return ama_status_success();
}
/** Execute one bounded native operation; every temporary owner is released before return. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state, const ama_t_5d438dc4f881420d2ec594167826b6aae34eb1581498f3c7891ece0766802b76 *parameters,
    ama_t_71e4bd6c27c34ad91367f604b0115e328aa07656cee797c21e869489ff10716e *results, const AmaNativeServices *services, AmaStatus *status) {
    (void)parameters; (void)results; (void)services;
    if (state->lease.resource == NULL) { *status = ama_status_make(AMA_INVALID_STATE, NULL, 0); return; }
    AmaNativeEventCompletion completion = {0};
    bool available = false;
    *status = ama_native_event_service_take(state->lease.resource, &completion, &available);
    if (status->code == AMA_OK && available) { *status = ama_n_e34937e219f84be1f1e0c364ad6e2333fd58e60295c27110ea97a69a88ac4904_publish(results->AMA_OUTPUT(0), &completion); }
    ama_native_event_completion_drop(&completion);
}
#include "ama_n_be2e35e01b9d693789bf50198841b52f867e802f5dc7d6dedf45b323afe7235d.end.h"
