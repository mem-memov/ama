/** @file
 * @brief Notify the host from a real child C helper while both Ama contexts are active.
 * [EMBED-08](../../../../../../spec/implementation/c-library.md#embed-08) requires
 * this ordinary C helper to return before the child reaches its next yield.
 */
#include "probe-api.h"
#include AMA_PROBE_INTERFACE_HEADER
#include AMA_PROBE_BEGIN_HEADER
#include "../../probe.h"
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *state) {
    state->unused=false; return ama_status_success();
}
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) {
    (void)state; return ama_status_success();
}
AmaStatus AMA_NATIVE(clone)(AMA_NATIVE(State) *destination,const AMA_NATIVE(State) *source) {
    *destination=*source; return ama_status_success();
}
/** This is an actual ordinary C call interval, not a stored continuation frame. */
__attribute__((noinline)) static void AMA_NATIVE(signal)(void) {
    ama_nested_probe_v1_event(AMA_NESTED_HELPER_ENTER);
    ama_nested_probe_v1_event(AMA_NESTED_HELPER_LEAVE);
}
/** Native entry is inlined in the child dispatcher; the helper remains ordinary C. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const AMA_PROBE_PARAMETERS *parameters,AMA_PROBE_RESULTS *results,
    const AmaNativeServices *services,AmaStatus *status) {
    (void)state; (void)parameters; (void)results; (void)services;
    AMA_NATIVE(signal)(); *status=ama_status_success();
}
#include AMA_PROBE_END_HEADER
