/** @file
 * @brief Print complete owned failure descriptions through their declared text protocol.
 */
#include "ama_n_b94f625519c534e00b6217d0e0de68e8cafc3af7b75a0a7b5a2af39e4c3696f9.interface.h"
#include "ama_n_b94f625519c534e00b6217d0e0de68e8cafc3af7b75a0a7b5a2af39e4c3696f9.begin.h"
#include <stdio.h>
/** Borrow explicit bytes only through this activation, including empty text. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_2426fcf3760e9341f2957d58c732b9e0a9e2203bc86a21f4999d2bd2be5aec36 *parameters,
    ama_t_a820aaf7c5e3a58361c6bb29bf3197355a15a42051706599b375ea0ffaea3ce9 *results,
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
#include "ama_n_b94f625519c534e00b6217d0e0de68e8cafc3af7b75a0a7b5a2af39e4c3696f9.end.h"
