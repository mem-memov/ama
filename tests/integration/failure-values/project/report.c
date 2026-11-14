/** @file
 * @brief Print complete owned failure descriptions through their declared text protocol.
 */
#include "ama_n_438eabf60c126290027eacf1be199d9eea346f928d792f91183a62cd79e02b74.interface.h"
#include "ama_n_438eabf60c126290027eacf1be199d9eea346f928d792f91183a62cd79e02b74.begin.h"
#include <stdio.h>
/** Borrow explicit bytes only through this activation, including empty text. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_4989273ef4424ad19452b8976b898b8e7c17284ddb1ecb6b0c94617b73992a56 *parameters,
    ama_t_02fea466bec63d6bf3f437d9be2eee4fd6cef1853c2334d2606f82b497ab4e96 *results,
    const AmaNativeServices *services, AmaStatus *status) {
    (void)state; (void)results; (void)services;
    AmaByteView description;
    *status = ama_n_762bf961da08fb6cb5c3d7150457175890b330d62849df81b283e451413eef10_view(parameters->AMA_INPUT(0)->payload, &description);
    if (status->code != AMA_OK) { return; }
    fputs("description=", stdout);
    if (description.length != 0) { fwrite(description.data, 1, description.length, stdout); }
    putchar('\n');
}
/** Construct a fresh resource-free observer, without any hidden text ownership. */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *destination) {
    *destination = (AMA_NATIVE(State)){0};
    return ama_status_success();
}
/** Declared input fields, not the native payload, own text value resources. */
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) {
    (void)state;
    return ama_status_success();
}
#include "ama_n_438eabf60c126290027eacf1be199d9eea346f928d792f91183a62cd79e02b74.end.h"
