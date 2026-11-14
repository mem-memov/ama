/** @file
 * @brief Publish coherent expected-failure outcomes without swallowing technical status.
 * [ERROR-01](../../spec/implementation/diagnostics.md#error-01) requires one
 * condition/description value; [CABI-05](../../spec/implementation/native-interface.md#cabi-05)
 * keeps description ownership out of opaque native payloads.
 */
#include "ama_n_edcb3e348eb3d3a06b885e12e77fcb6bd0b86d265d2077a038d757a4f799f515.interface.h"
#include "ama_n_edcb3e348eb3d3a06b885e12e77fcb6bd0b86d265d2077a038d757a4f799f515.begin.h"
/** Establish description first, then publish the discriminator without any
 * intervening Ama execution. Failed text preparation preserves the prior value.
 */
AmaStatus AMA_NATIVE(set)(ama_t_e5d6b896a3da05425909140ebbbfebcc448cea3c969ff7626c1e9c8596bd635e *destination, AmaByteView description) {
    AmaStatus status = ama_n_762bf961da08fb6cb5c3d7150457175890b330d62849df81b283e451413eef10_replace(destination->AMA_OUTPUT(0)->payload, description);
    if (status.code == AMA_OK) { destination->payload->failed = true; }
    return status;
}
/** Empty replacement allocates nothing; no previous failure text can survive. */
AmaStatus AMA_NATIVE(clear)(ama_t_e5d6b896a3da05425909140ebbbfebcc448cea3c969ff7626c1e9c8596bd635e *destination) {
    AmaStatus status = ama_n_762bf961da08fb6cb5c3d7150457175890b330d62849df81b283e451413eef10_replace(destination->AMA_OUTPUT(0)->payload, (AmaByteView){0});
    if (status.code == AMA_OK) { destination->payload->failed = false; }
    return status;
}
/** Entry resets the result interface before this body, then clears presence. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_7c69e8a0795ae293f8e2030834dbdb1ba0768f4597344cef1a5b5b1647510e57 *parameters,
    ama_t_e5a3ad7a9570eb251fc352b832e670791da3c44f378d56708667132478effa9f *results,
    const AmaNativeServices *services, AmaStatus *status) {
    (void)parameters; (void)results; (void)services;
    state->failed = false;
    *status = ama_status_success();
}
/** Native initialization follows successful generated description construction. */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *destination) {
    *destination = (AMA_NATIVE(State)){0};
    return ama_status_success();
}
/** Generated ownership alone destroys the text result. */
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) {
    (void)state;
    return ama_status_success();
}
/** Source discriminator and separately cloned description form one copied value. */
AmaStatus AMA_NATIVE(clone)(AMA_NATIVE(State) *destination, const AMA_NATIVE(State) *source) {
    *destination = *source;
    return ama_status_success();
}
/** Presence is declared state, not a test of message wording or byte length. */
AmaStatus AMA_NATIVE(to_condition)(const AMA_NATIVE(State) *state, bool *output) {
    *output = state->failed;
    return ama_status_success();
}
#include "ama_n_edcb3e348eb3d3a06b885e12e77fcb6bd0b86d265d2077a038d757a4f799f515.end.h"
