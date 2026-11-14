/** @file
 * @brief Construct a condition-capable failure through a normal typed module invocation.
 * [ERROR-01](../../../spec/implementation/diagnostics.md#error-01) lets pure
 * Ama callers create an owned failure value without a separate presence result.
 */
#include "ama_n_2066fb8ff9b20aa0fda03898a004d946281fe18e17dbd02fc4e9214c0c8b272b.interface.h"
#include "ama_n_2066fb8ff9b20aa0fda03898a004d946281fe18e17dbd02fc4e9214c0c8b272b.begin.h"
/** Copy the borrowed input into the output's owned description before publishing
 * presence. A technical allocation failure returns unchanged as native status.
 */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_149943b26c759aec1c56995e039738186f9368e53d728b379763f67a5cd8ad04 *parameters,
    ama_t_bb90f68c826cf638d4e04f02d14eb9fc69ad679a9da979426da9aa3c7dc6deb3 *results,
    const AmaNativeServices *services, AmaStatus *status) {
    (void)state; (void)services;
    AmaByteView description;
    *status = ama_n_762bf961da08fb6cb5c3d7150457175890b330d62849df81b283e451413eef10_view(parameters->AMA_INPUT(0)->payload, &description);
    if (status->code != AMA_OK) { return; }
    *status = ama_n_edcb3e348eb3d3a06b885e12e77fcb6bd0b86d265d2077a038d757a4f799f515_set(results->AMA_OUTPUT(0), description);
}
/** The constructor's own payload owns no resource or borrowed view. */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *destination) {
    *destination = (AMA_NATIVE(State)){0};
    return ama_status_success();
}
/** Generated finalization owns every declared input/result resource. */
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) {
    (void)state;
    return ama_status_success();
}
/** Copy only the resource-free payload; generated cloning copies declared fields. */
AmaStatus AMA_NATIVE(clone)(AMA_NATIVE(State) *destination, const AMA_NATIVE(State) *source) {
    *destination = *source;
    return ama_status_success();
}
#include "ama_n_2066fb8ff9b20aa0fda03898a004d946281fe18e17dbd02fc4e9214c0c8b272b.end.h"
