/** @file
 * @brief Observe canonical integer results through the real generated native interface.
 */
#include "ama_n_ab76f6f6a1f801f3945d320efc39ab984e216f70a49be376946442a66fa6ee1c.interface.h"
#include "ama_n_ab76f6f6a1f801f3945d320efc39ab984e216f70a49be376946442a66fa6ee1c.begin.h"
#include <stdio.h>
/** Convert the borrowed integer through its declared protocol before printing. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_2e22ef69261bee3ab284ffbc153d6c3fb0890fde66e778101e9cfbb9b0e23a07 *parameters, ama_t_c384733dfd60ef19e9dbd3082ded5011961b4a96ddb90fed978b6fe3c7757fab *results,
    const AmaNativeServices *services, AmaStatus *status) {
    (void)state; (void)results; (void)services;
    size_t number = 0;
    *status = ama_n_1ad46fade0175c4e31ef9315b33c02e79bb0f66fd54f938c04bd36d8838b2a42_to_index(parameters->AMA_INPUT(0)->payload, &number);
    if (status->code == AMA_OK) { printf("length=%zu\n", number); }
}
/** Construct the resource-free observer payload in fresh storage. */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *destination) {
    *destination = (AMA_NATIVE(State)){0};
    return ama_status_success();
}
/** Release no external resources and retain no borrowed integer view. */
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) {
    (void)state;
    return ama_status_success();
}
#include "ama_n_ab76f6f6a1f801f3945d320efc39ab984e216f70a49be376946442a66fa6ee1c.end.h"
