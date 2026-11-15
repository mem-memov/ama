/** @file
 * @brief Publish ordinary retained task phases without executing or enrolling a task.
 * [ASYNC-03](../../../spec/language/async-and-parallelism.md#async-03)
 * keeps exactly one phase separate from completion data and runtime host outcomes.
 */
#include "ama_n_3f995290b378db1c82dbfdb55c99332bfe86d8ee249afbcfca803ce322998c66.interface.h"
#include "ama_n_3f995290b378db1c82dbfdb55c99332bfe86d8ee249afbcfca803ce322998c66.begin.h"
#include "phase+/native/selection.h"
/** Publish the prepared one-hot tuple without allocation, callbacks or intervening execution. */
AmaStatus AMA_NATIVE(assign)(ama_t_f14ae70b65c2b1e0e969f4ae06aed3cc57e808f832f94294aac1f39e559cc930 *destination, unsigned kind) {
    bool predicates[3];
    if (!AMA_NATIVE(select)(kind, predicates)) {
        return (AmaStatus){AMA_NATIVE_FAILURE, "Invalid task phase", 18};
    }
    destination->AMA_OUTPUT(0)->payload->value = predicates[0];
    destination->AMA_OUTPUT(1)->payload->value = predicates[1];
    destination->AMA_OUTPUT(2)->payload->value = predicates[2];
    return ama_status_success();
}
/** Entry resets declared results to their default Ready phase; it executes no task. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_bb5107a29d729307ef8c3161ec3325c24340c9432612982877d8d6b2531a1306 *parameters, ama_t_b1c12e37e68360634f6ace72e03342908d1b0990889fbed2fa8cb9894c880aa4 *results,
    const AmaNativeServices *services, AmaStatus *status) {
    (void)state; (void)parameters; (void)results; (void)services;
    *status = ama_status_success();
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
#include "ama_n_3f995290b378db1c82dbfdb55c99332bfe86d8ee249afbcfca803ce322998c66.end.h"
