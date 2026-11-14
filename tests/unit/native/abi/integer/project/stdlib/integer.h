/**
 * @file
 * @brief Bind the maintained integer representation to concrete empty test entry views.
 *
 * The actual standard-root representation and protocol declarations are borrowed
 * from the library; this harness supplies only its isolated empty entry views.
 * Its empty entry views are this harness's concrete nominal target binding.
 */
#ifndef AMA_INTEGER_PROTOCOL_FIXTURE_H
#define AMA_INTEGER_PROTOCOL_FIXTURE_H
#include "native/abi/bytes.h"
#include "native/abi/services.h"
#include "native/toolchain/gcc.h"

/** Own little-endian base-256 magnitude limbs; live values have at least one limb.
 * Zero is canonical nonnegative. No Ama handle or borrowed view occupies this payload.
 */
typedef struct AMA_NATIVE(State) AMA_NATIVE(State);
#include "../../../../../../../stdlib/modules/integer.h"

/** Nominal empty input interface; reserved avoids a nonstandard empty C struct. */
typedef struct { unsigned char reserved; } AMA_NATIVE(Inputs);
/** Nominal empty result interface; integer conversion reads native state instead. */
typedef struct { unsigned char reserved; } AMA_NATIVE(Outputs);

/** Execute the fixture's explicitly empty native body without altering integer state. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const AMA_NATIVE(Inputs) *inputs, AMA_NATIVE(Outputs) *outputs,
    const AmaNativeServices *services, AmaStatus *status) {
    (void)state; (void)inputs; (void)outputs; (void)services;
    *status = ama_status_success();
}
#endif
