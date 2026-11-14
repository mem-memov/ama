/** @file
 * @brief Demonstrate synchronous file failure as an ordinary owned Ama value.
 * [ERROR-01](../../../../spec/implementation/diagnostics.md#error-01) separates
 * expected operating-system failures from technical construction errors.
 */
#include "ama_n_6a7f6a90acd415cc528de4e6176794c1fcbeccb986d00a42da8af620998d091d.interface.h"
#include "ama_n_6a7f6a90acd415cc528de4e6176794c1fcbeccb986d00a42da8af620998d091d.begin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/** Open and close within this activation. Borrowed text never escapes; allocation
 * failure is technical, while malformed paths and open/close failures are data.
 */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_9829e45a81b133526d148c801167f49dc672642fb6666f4fc2ca91fc589e5dea *parameters,
    ama_t_b9dc3c72d847309fe1cb9a397519ba18a3500c58e11c891d800c8e44eac327b4 *results,
    const AmaNativeServices *services, AmaStatus *status) {
    (void)state; (void)services;
    AmaByteView path;
    *status = ama_n_762bf961da08fb6cb5c3d7150457175890b330d62849df81b283e451413eef10_view(parameters->AMA_INPUT(0)->payload, &path);
    if (status->code != AMA_OK) { return; }
    const char *failure = NULL;
    if (path.length == 0 || memchr(path.data, 0, path.length) != NULL) {
        failure = "Invalid file path";
    } else {
        if (path.length == SIZE_MAX) { *status = (AmaStatus){11, "Path size overflow", 18}; return; }
        char *terminated = malloc(path.length + 1);
        if (terminated == NULL) { *status = (AmaStatus){2, "Path allocation failed", 22}; return; }
        memcpy(terminated, path.data, path.length);
        terminated[path.length] = 0;
        FILE *file = fopen(terminated, "rb");
        free(terminated);
        if (file == NULL) { failure = "Cannot open file"; }
        else if (fclose(file) != 0) { failure = "Cannot close file"; }
    }
    if (failure != NULL) {
        AmaByteView message = {(const unsigned char *)failure, strlen(failure)};
        *status = ama_n_edcb3e348eb3d3a06b885e12e77fcb6bd0b86d265d2077a038d757a4f799f515_set(results->AMA_OUTPUT(0), message);
    }
}
/** Initialize the resource-free native payload; generated fields own all values. */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *destination) {
    *destination = (AMA_NATIVE(State)){0};
    return ama_status_success();
}
/** No stream survives entry, so finalization owns no external resource. */
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) {
    (void)state;
    return ama_status_success();
}
#include "ama_n_6a7f6a90acd415cc528de4e6176794c1fcbeccb986d00a42da8af620998d091d.end.h"
