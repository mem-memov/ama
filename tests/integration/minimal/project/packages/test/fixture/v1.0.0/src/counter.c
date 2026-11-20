/** @file
 * @brief Execute an address-sensitive retained counter and release every independent native resource exactly once.
 * [LIFE-01](../../../../../../../../../spec/implementation/native-interface.md#life-01) gives
 * successful default/clone construction one destruction credit. The generated
 * naming context and entry signature come from the real authoring scaffold;
 * authored helpers use no persistent Ama handles or borrowed interface views.
 */
#include "ama_n_33af73c8e34c890609f3be9220619411a26d0024f3489dd53d3d4c37b523d74c.interface.h"
#include "ama_n_33af73c8e34c890609f3be9220619411a26d0024f3489dd53d3d4c37b523d74c.begin.h"
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
    const ama_t_048e1e047a99916612c26f937f7f4696103caa29287e2d87484db8be03d84cc6 *parameters,
    ama_t_b6c234bf53233e1bf773e8f16dc0a97fd53585f4d2c44a439d4fc2d2d2e5a6ab *results,
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

#include "ama_n_33af73c8e34c890609f3be9220619411a26d0024f3489dd53d3d4c37b523d74c.end.h"
