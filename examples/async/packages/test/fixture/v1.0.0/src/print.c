/** @file
 * @brief Write one counted borrowed text value synchronously, checking output before returning to Ama.
 * [STACK-03](../../../../../../../spec/implementation/compiler.md#stack-03) requires native
 * completion before Ama continues; [CABI-05](../../../../../../../spec/implementation/native-interface.md#cabi-05)
 * forbids retaining the parameter's bytes after this operation.
 */
#include "ama_n_ff91594cd9067ea073a248353a1893dc9b95235f2aa69d113bbd0a0f15596ae6.interface.h"
#include "ama_n_ff91594cd9067ea073a248353a1893dc9b95235f2aa69d113bbd0a0f15596ae6.begin.h"
#include <stdio.h>
/** Construct no native resource; declared fields own the parameter value. */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *state) { *state = (AMA_NATIVE(State)){0}; return ama_status_success(); }
/** There is no retained output buffer or callback to release. */
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) { (void)state; return ama_status_success(); }
/** Print and flush before returning; an I/O failure is technical and never masquerades as a task outcome. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state, const ama_t_01326e381fd9344a0e7bb99e3fd9d1b0b66d8e4e8d086b6add5bdcbbf0da33b8 *parameters,
    ama_t_d7e3062029664faa99fa06bffb6c8ec522e15d7a548a54b1939d569731d2a026 *results, const AmaNativeServices *services, AmaStatus *status) {
    (void)state; (void)results; (void)services;
    AmaByteView text;
    *status = ama_n_762bf961da08fb6cb5c3d7150457175890b330d62849df81b283e451413eef10_view(parameters->AMA_INPUT(0)->payload, &text);
    if (status->code != AMA_OK) { return; }
    if ((text.length != 0 && fwrite(text.data, 1, text.length, stdout) != text.length)
        || fputc('\n', stdout) == EOF || fflush(stdout) == EOF) {
        *status = ama_status_make(AMA_IO_FAILURE, NULL, 0);
    }
}
#include "ama_n_ff91594cd9067ea073a248353a1893dc9b95235f2aa69d113bbd0a0f15596ae6.end.h"
