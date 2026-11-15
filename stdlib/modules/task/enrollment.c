/** @file
 * @brief Provide a noncopyable enrollment owner with all scheduling relationships kept in declared Ama fields.
 * [ASYNC-10](../../../spec/language/async-and-parallelism.md#async-10)
 * permits copying task values, not duplicating scheduling authority. The missing
 * clone capability enforces this boundary before any owned-tree clone begins.
 */
#include "ama_n_8ed1a75e785d180669239d66d027e1999129279b8430f5015178f058bdedf533.interface.h"
#include "ama_n_8ed1a75e785d180669239d66d027e1999129279b8430f5015178f058bdedf533.begin.h"
/** Default construction creates only this Ama field identity; no external resource or hidden task reference is allocated. */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *state) { *state = (AMA_NATIVE(State)){0}; return ama_status_success(); }
/** Enrollment relations are declared fields of the owning record; this payload owns no independent cleanup callback. */
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) { (void)state; return ama_status_success(); }
/** Ordinary entry neither grants scheduling rights nor resets an epoch; explicit record construction controls enrollment. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state, const ama_t_edfb9336ed12636c05892cb607c5886405a757528e81e336d9fc0e39d7ba420d *parameters,
    ama_t_269c5b01e1dc44955aa3eda34d448e73378d9755a37a79f119329668ca01987a *results, const AmaNativeServices *services, AmaStatus *status) {
    (void)state; (void)parameters; (void)results; (void)services;
    *status = ama_status_success();
}
#include "ama_n_8ed1a75e785d180669239d66d027e1999129279b8430f5015178f058bdedf533.end.h"
