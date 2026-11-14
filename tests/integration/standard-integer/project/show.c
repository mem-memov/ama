/** @file
 * @brief Observe canonical integer results through the real generated native interface.
 */
#include "ama_n_3e877baa4e12fdd9aa990580ae5cb299a211c35d314205da540ebc81f05c57ec.interface.h"
#include "ama_n_3e877baa4e12fdd9aa990580ae5cb299a211c35d314205da540ebc81f05c57ec.begin.h"
#include <stdio.h>
/** Convert the borrowed integer through its declared protocol before printing. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_fe1ca4f72eae68567664301e392c97a433f86b3d9c2fed8f760a529a81e573be *parameters, ama_t_000c372c5cb87045c1c07dd1e3889efff5476fa71e1b1a4db28a4f00e866c714 *results,
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
#include "ama_n_3e877baa4e12fdd9aa990580ae5cb299a211c35d314205da540ebc81f05c57ec.end.h"
