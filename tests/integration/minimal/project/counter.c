/** @file
 * @brief Execute an address-sensitive retained counter and release every independent native resource exactly once.
 * [LIFE-01](../../../../spec/implementation/native-interface.md#life-01) gives
 * successful default/clone construction one destruction credit. The generated
 * naming context and entry signature come from the real authoring scaffold;
 * authored helpers use no persistent Ama handles or borrowed interface views.
 */
#include "ama_n_cff6ead0c2e563219f2a7525b83b3a9a43dd57b2b6d624cd414d15e03a1acf7e.interface.h"
#include "ama_n_cff6ead0c2e563219f2a7525b83b3a9a43dd57b2b6d624cd414d15e03a1acf7e.begin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** Check native resource ownership at its original final address, including both ends of its private allocation. */
static bool AMA_NATIVE(valid)(const AMA_NATIVE(State) *state) {
    return state->address == (uintptr_t)state && state->resource != NULL
        && state->resource[0] == 0xA5 && state->resource[15] == 0xA5;
}

/** Construct only fresh storage; allocation failure owns no resource and needs no destructor callback. */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *destination) {
    *destination = (AMA_NATIVE(State)){.address = (uintptr_t)destination};
    destination->resource = malloc(16);
    if (destination->resource == NULL) { return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0); }
    memset(destination->resource, 0xA5, 16);
    return ama_status_success();
}

/** Preserve the logical count with an independent allocation and final self-address, never a shallow resource copy. */
AmaStatus AMA_NATIVE(clone)(AMA_NATIVE(State) *destination, const AMA_NATIVE(State) *source) {
    if (!AMA_NATIVE(valid)(source)) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    AmaStatus status = AMA_NATIVE(initialize)(destination);
    if (status.code == AMA_OK) { destination->calls = source->calls; }
    return status;
}

/** Flush each native effect before returning to Ama so a stream error is reported by this completed C activation. */
static AmaStatus AMA_NATIVE(report)(const char *label, size_t calls) {
    return printf("%s=%zu\n", label, calls) < 0 || fflush(stdout) != 0
        ? ama_status_make(AMA_IO_FAILURE, NULL, 0) : ama_status_success();
}

/** Return synchronously after one visible increment; overflow or output failure becomes a technical status before Ama can continue. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_524c00298ce5cfc008d910eb885d37b99f42afd4b24f025050a805db7b0cbe23 *parameters,
    ama_t_dfcf30136d35bf5c74120d56740c04c3ce8e278689353addbf10302d2626fac6 *results,
    const AmaNativeServices *services, AmaStatus *status) {
    (void)parameters;
    (void)results;
    (void)services;
    if (!AMA_NATIVE(valid)(state)) { *status = ama_status_make(AMA_INVALID_STATE, NULL, 0); return; }
    if (state->calls == SIZE_MAX) { *status = ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0); return; }
    state->calls++;
    *status = AMA_NATIVE(report)("count", state->calls);
}

/** Consume the native resource before reporting cleanup; a failed output never authorizes a second destruction attempt. */
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) {
    bool valid = AMA_NATIVE(valid)(state);
    size_t calls = state->calls;
    free(state->resource);
    state->resource = NULL;
    if (!valid) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    return AMA_NATIVE(report)("drop", calls);
}

#include "ama_n_cff6ead0c2e563219f2a7525b83b3a9a43dd57b2b6d624cd414d15e03a1acf7e.end.h"
