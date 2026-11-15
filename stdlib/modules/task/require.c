/** @file
 * @brief Validate explicit scheduler protocol predicates before continuing execution.
 * [ASYNC-07](../../../spec/language/async-and-parallelism.md#async-07)
 * requires finite positive budgets and rejects states outside its task/wait protocol.
 * Failure is technical; it never fabricates readiness or a successful task outcome.
 */
#include "ama_n_0e93fb3502b9d5ba90d316eeb1e656d04e0959f0956fdd22e6cecf43853d5d2b.interface.h"
#include "ama_n_0e93fb3502b9d5ba90d316eeb1e656d04e0959f0956fdd22e6cecf43853d5d2b.begin.h"
/** Validation owns no native resource or hidden scheduler relationship. */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *state) { *state = (AMA_NATIVE(State)){0}; return ama_status_success(); }
/** Retire no resource; all borrowed inputs expire with their activation. */
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) { (void)state; return ama_status_success(); }
/** Resource-free validation modules can be copied independently. */
AmaStatus AMA_NATIVE(clone)(AMA_NATIVE(State) *destination, const AMA_NATIVE(State) *source) { *destination = *source; return ama_status_success(); }
/** Check the declared input through its typed view without retaining any native/Ama borrow. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state, const ama_t_b540a241dc44753f9c8dc1e0a80e9f27e948adf968be05a3dad9e73258ebb3f0 *parameters,
    ama_t_4fa0aca991cd638e898235ce81a7f12fe9197c752c2b73f9ff3686fec33e787a *results, const AmaNativeServices *services, AmaStatus *status) {
    (void)state; (void)results; (void)services;
    if (parameters->AMA_INPUT(0)->payload->value != 1) {
        static const char message[] = "Task scheduler protocol invariant failed";
        *status = ama_status_make(AMA_INVALID_STATE, message, sizeof(message) - 1);
    } else { *status = ama_status_success(); }
}
#include "ama_n_0e93fb3502b9d5ba90d316eeb1e656d04e0959f0956fdd22e6cecf43853d5d2b.end.h"
