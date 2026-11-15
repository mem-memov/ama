/** @file
 * @brief Construct the ordinary completed phase without entering or resuming a task.
 * [ASYNC-03](../../../../spec/language/async-and-parallelism.md#async-03)
 * makes the phase a typed value independent of application failure or scheduling.
 */
#include "ama_n_c8ceeaca395273d724c3f527eac90c74691a3f9397f1bec93a8813ab925b3854.interface.h"
#include "ama_n_c8ceeaca395273d724c3f527eac90c74691a3f9397f1bec93a8813ab925b3854.begin.h"
/** Publish a completed phase in the fully constructed result; retain no borrowed state. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_c2178d147065c7fb190fee7f37dbc26848e77ca4a37ac4fd308ef9e11d69c247 *parameters, ama_t_187362f5d4c310f887dd53aaebe4ef857f237afd8d9e48f22248c0811c67c361 *results,
    const AmaNativeServices *services, AmaStatus *status) {
    (void)state; (void)parameters; (void)results; (void)services;
    *status = ama_n_3f995290b378db1c82dbfdb55c99332bfe86d8ee249afbcfca803ce322998c66_assign(results->AMA_OUTPUT(0), ama_n_3f995290b378db1c82dbfdb55c99332bfe86d8ee249afbcfca803ce322998c66_COMPLETED);
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
#include "ama_n_c8ceeaca395273d724c3f527eac90c74691a3f9397f1bec93a8813ab925b3854.end.h"
