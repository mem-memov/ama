/** @file
 * @brief Construct the ordinary ready phase without entering or resuming a task.
 * [ASYNC-03](../../../../spec/language/async-and-parallelism.md#async-03)
 * makes the phase a typed value independent of application failure or scheduling.
 */
#include "ama_n_14b02f317cce05a16bedf29d09a304b556b739fcbb5b4b9bdabc9ec6ce499edf.interface.h"
#include "ama_n_14b02f317cce05a16bedf29d09a304b556b739fcbb5b4b9bdabc9ec6ce499edf.begin.h"
/** Publish a ready phase in the fully constructed result; retain no borrowed state. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_ace8710e1ffd75586b714688cc26375916e0354b1d52316ecbeaddfcb0718880 *parameters, ama_t_a2b894156c269e1677f6f7f80f4c87a7c5c75616033aa1a06a0fb5e0d9085cb5 *results,
    const AmaNativeServices *services, AmaStatus *status) {
    (void)state; (void)parameters; (void)results; (void)services;
    *status = ama_n_3f995290b378db1c82dbfdb55c99332bfe86d8ee249afbcfca803ce322998c66_assign(results->AMA_OUTPUT(0), ama_n_3f995290b378db1c82dbfdb55c99332bfe86d8ee249afbcfca803ce322998c66_READY);
}
/** Initialize only the resource-free payload; generated fields own predicates. */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *destination) {
    *destination = (AMA_NATIVE(State)){0};
    return ama_status_success();
}
/** Predicate fields are destroyed by ordinary generated ownership cleanup. */
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) {
    (void)state;
    return ama_status_success();
}
/** Copy only the empty native payload; generated cloning copies predicate fields. */
AmaStatus AMA_NATIVE(clone)(AMA_NATIVE(State) *destination, const AMA_NATIVE(State) *source) {
    *destination = *source;
    return ama_status_success();
}
#include "ama_n_14b02f317cce05a16bedf29d09a304b556b739fcbb5b4b9bdabc9ec6ce499edf.end.h"
