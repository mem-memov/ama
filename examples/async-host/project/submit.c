/** @file
 * @brief Submit a borrowed operation's native rights to an independently resident backend before the Ama entry returns.
 * [ASYNC-05](../../../spec/language/async-and-parallelism.md#async-05)
 * allows native callbacks only; [CABI-05](../../../spec/implementation/native-interface.md#cabi-05)
 * forbids retaining the temporary operation view in that backend.
 */
#include "ama_n_8cef89394130e7dce2bfa11c6ecb446db49fdbe2dfd01e48be47ff56f5720cda.interface.h"
#include "ama_n_8cef89394130e7dce2bfa11c6ecb446db49fdbe2dfd01e48be47ff56f5720cda.begin.h"
#include "backend-native.h"
/** Payload construction owns no external worker or callback. */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *state) { *state = (AMA_NATIVE(State)){0}; return ama_status_success(); }
/** The supported native host acknowledges jobs independently after context disposal. */
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) { (void)state; return ama_status_success(); }
/** Pass an endpoint borrow synchronously; the backend obtains its own endpoint and byte credits before returning. */
static AmaStatus AMA_NATIVE(submit)(const ama_t_3211b272d4ffc58b35668036530071d3838391fee0aa5c4208851404f0010b09 *operation,
    void *resource) {
    return backend_submit(&operation->payload->submission, *(const bool *)resource);
}
/** Validate the explicit mode and return normally without retaining this C activation across the root yield. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state, const ama_t_de061baaab5fc37abe61617f107e0e10a1dab4651f04474941b8575b4ab75a9f *parameters,
    ama_t_a65acc7301787d5380c29ae705eb48498e7f9248ea80f07135f1d09ada7b2391 *results, const AmaNativeServices *services, AmaStatus *status) {
    (void)state; (void)results; (void)services;
    if (parameters->AMA_INPUT(1)->payload->value > 1) { *status = ama_status_make(AMA_INVALID_STATE, NULL, 0); return; }
    bool immediate = parameters->AMA_INPUT(1)->payload->value != 0;
    *status = parameters->AMA_INPUT(0)->with_read(parameters->AMA_INPUT(0)->storage, AMA_NATIVE(submit), &immediate);
}
#include "ama_n_8cef89394130e7dce2bfa11c6ecb446db49fdbe2dfd01e48be47ff56f5720cda.end.h"
