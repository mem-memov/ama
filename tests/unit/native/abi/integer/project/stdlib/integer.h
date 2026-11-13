/**
 * @file
 * @brief Declare the integer protocol fixture with owned base-256 magnitude limbs.
 *
 * The fixture's declared library identity is ..integer. C acceptance uses an
 * isolated fixed namespace; standard-root frontend wiring is a later consumer.
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
typedef struct {
    unsigned char *limbs;
    size_t count;
    bool negative;
    bool live;
} AMA_NATIVE(State);

/** Nominal empty input interface; reserved avoids a nonstandard empty C struct. */
typedef struct { unsigned char reserved; } AMA_NATIVE(Inputs);
/** Nominal empty result interface; integer conversion reads native state instead. */
typedef struct { unsigned char reserved; } AMA_NATIVE(Outputs);

/** Construct native zero in fresh storage through the selected default callback. */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *);
/** Release one live native value exactly once and leave finalized empty storage. */
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *);
/** Parse optional minus followed by decimal digits into independently owned limbs.
 * Invalid/failed construction releases its prefix and leaves non-live storage.
 */
AmaStatus AMA_NATIVE(from_literal)(AMA_NATIVE(State) *, AmaByteView);
/** Clone independent native magnitude storage from a distinct live source. */
AmaStatus AMA_NATIVE(clone)(AMA_NATIVE(State) *, const AMA_NATIVE(State) *);
/** Inspect current mathematical nonzero state without executing the empty entry. */
AmaStatus AMA_NATIVE(to_condition)(const AMA_NATIVE(State) *, bool *);
/** Convert a nonnegative host-representable magnitude; failure preserves output. */
AmaStatus AMA_NATIVE(to_index)(const AMA_NATIVE(State) *, size_t *);
/** Construct every host size exactly, without default/literal dispatch or decimal bytes. */
AmaStatus AMA_NATIVE(from_size)(AMA_NATIVE(State) *, size_t);

/** Execute the fixture's explicitly empty native body without altering integer state. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const AMA_NATIVE(Inputs) *inputs, AMA_NATIVE(Outputs) *outputs,
    const AmaNativeServices *services, AmaStatus *status) {
    (void)state; (void)inputs; (void)outputs; (void)services;
    *status = ama_status_success();
}
#endif
