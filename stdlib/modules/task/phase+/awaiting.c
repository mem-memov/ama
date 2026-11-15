/** @file
 * @brief Construct the ordinary awaiting phase without entering or resuming a task.
 * [ASYNC-03](../../../../spec/language/async-and-parallelism.md#async-03)
 * makes the phase a typed value independent of application failure or scheduling.
 */
#include "ama_n_fac64f4490c3c786b79f8e05f8d36630e000cb0089f16cb69baaef9772454a47.interface.h"
#include "ama_n_fac64f4490c3c786b79f8e05f8d36630e000cb0089f16cb69baaef9772454a47.begin.h"
/** Publish an awaiting phase in the fully constructed result; retain no borrowed state. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_d856c954a3692df251f374e3d3fd8465575a048f65b3ed0da7996792ced4f69c *parameters, ama_t_fc7424b0e8b9f5c49c7b8121d366ce1b2115de0f45d29f8c14a3814fcdae586e *results,
    const AmaNativeServices *services, AmaStatus *status) {
    (void)state; (void)parameters; (void)results; (void)services;
    *status = ama_n_3f995290b378db1c82dbfdb55c99332bfe86d8ee249afbcfca803ce322998c66_assign(results->AMA_OUTPUT(0), ama_n_3f995290b378db1c82dbfdb55c99332bfe86d8ee249afbcfca803ce322998c66_AWAITING);
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
#include "ama_n_fac64f4490c3c786b79f8e05f8d36630e000cb0089f16cb69baaef9772454a47.end.h"
