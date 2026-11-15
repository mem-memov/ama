/** @file
 * @brief Validate positive finite host-sized scheduling limits before continuing execution.
 * [ASYNC-07](../../../../spec/language/async-and-parallelism.md#async-07)
 * requires finite positive budgets and rejects states outside its task/wait protocol.
 * Failure is technical; it never fabricates readiness or a successful task outcome.
 */
#include "ama_n_390d7d98ef7da307b31b4e493899a4ce9e452526154fac0a5b3abc6621be5dad.interface.h"
#include "ama_n_390d7d98ef7da307b31b4e493899a4ce9e452526154fac0a5b3abc6621be5dad.begin.h"
/** Validation owns no native resource or hidden scheduler relationship. */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *state) { *state = (AMA_NATIVE(State)){0}; return ama_status_success(); }
/** Retire no resource; all borrowed inputs expire with their activation. */
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) { (void)state; return ama_status_success(); }
/** Resource-free validation modules can be copied independently. */
AmaStatus AMA_NATIVE(clone)(AMA_NATIVE(State) *destination, const AMA_NATIVE(State) *source) { *destination = *source; return ama_status_success(); }
/** Check the declared input through its typed view without retaining any native/Ama borrow. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state, const ama_t_98aff4f81726facd801ef568c8cb873626fb51909facb47f5df36e78ea203e48 *parameters,
    ama_t_f0d1ec2b0b478348cc8d35dca48ff20a85baaff667b67e16d1c99b7334b776f9 *results, const AmaNativeServices *services, AmaStatus *status) {
    (void)state; (void)results; (void)services;
    size_t limit = 0;
    *status = ama_n_1ad46fade0175c4e31ef9315b33c02e79bb0f66fd54f938c04bd36d8838b2a42_to_index(parameters->AMA_INPUT(0)->payload, &limit);
    if (status->code != AMA_OK || limit == 0) {
        static const char message[] = "Scheduling budget must be positive and fit size_t";
        *status = ama_status_make(AMA_INVALID_STATE, message, sizeof(message) - 1);
    }
}
#include "ama_n_390d7d98ef7da307b31b4e493899a4ce9e452526154fac0a5b3abc6621be5dad.end.h"
