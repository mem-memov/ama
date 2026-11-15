/** @file
 * @brief Write one counted borrowed text value synchronously, checking output before returning to Ama.
 * [STACK-03](../../spec/implementation/compiler.md#stack-03) requires native
 * completion before Ama continues; [CABI-05](../../spec/implementation/native-interface.md#cabi-05)
 * forbids retaining the parameter's bytes after this operation.
 */
#include "ama_n_a61b00ccbb87762b11a76af3b1dce2570c740ac7c179c1a8e6a903ddbdec1d37.interface.h"
#include "ama_n_a61b00ccbb87762b11a76af3b1dce2570c740ac7c179c1a8e6a903ddbdec1d37.begin.h"
#include <stdio.h>
/** Construct no native resource; declared fields own the parameter value. */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *state) { *state = (AMA_NATIVE(State)){0}; return ama_status_success(); }
/** There is no retained output buffer or callback to release. */
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) { (void)state; return ama_status_success(); }
/** Print and flush before returning; an I/O failure is technical and never masquerades as a task outcome. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state, const ama_t_aee3655d2cffe15f02cb27e9c4b7bb408b12ab7f5ab4134d76d682e1e5782c4a *parameters,
    ama_t_32dea491bab63a4321929d754a9ce641554e708cf980cd87dd46ece1263c1d45 *results, const AmaNativeServices *services, AmaStatus *status) {
    (void)state; (void)results; (void)services;
    AmaByteView text;
    *status = ama_n_762bf961da08fb6cb5c3d7150457175890b330d62849df81b283e451413eef10_view(parameters->AMA_INPUT(0)->payload, &text);
    if (status->code != AMA_OK) { return; }
    if ((text.length != 0 && fwrite(text.data, 1, text.length, stdout) != text.length)
        || fputc('\n', stdout) == EOF || fflush(stdout) == EOF) {
        *status = ama_status_make(AMA_IO_FAILURE, NULL, 0);
    }
}
#include "ama_n_a61b00ccbb87762b11a76af3b1dce2570c740ac7c179c1a8e6a903ddbdec1d37.end.h"
