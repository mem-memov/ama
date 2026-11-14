/** @file
 * @brief Own an arbitrary-precision signed magnitude without hidden Ama references.
 * [PROTO-03](../../spec/implementation/native-interface.md#proto-03) requires
 * exact construction of every host size and read-only checked conversion.
 */
#ifndef AMA_STANDARD_INTEGER_STATE_H
#define AMA_STANDARD_INTEGER_STATE_H
#include "native/abi/bytes.h"
#include "native/abi/services.h"
/** Own canonical little-endian base-256 limbs; zero is nonnegative and live
 * values have at least one limb. Only successful constructors grant ownership.
 */
struct AMA_NATIVE(State) {
    unsigned char *limbs;
    size_t count;
    bool negative;
    bool live;
};
/** Construct zero in fresh storage; allocation failure leaves no live payload. */
AmaStatus AMA_NATIVE(initialize)(struct AMA_NATIVE(State) *);
/** Consume one live allocation exactly once. */
AmaStatus AMA_NATIVE(destroy)(struct AMA_NATIVE(State) *);
/** Parse optional minus and nonempty decimal digits; discard failed allocation. */
AmaStatus AMA_NATIVE(from_literal)(struct AMA_NATIVE(State) *, AmaByteView);
/** Copy mathematical data into independent fresh limb storage. */
AmaStatus AMA_NATIVE(clone)(struct AMA_NATIVE(State) *, const struct AMA_NATIVE(State) *);
/** Read mathematical nonzero without changing or executing the value. */
AmaStatus AMA_NATIVE(to_condition)(const struct AMA_NATIVE(State) *, bool *);
/** Return InvalidIndex for negative/too-large values, preserving output. */
AmaStatus AMA_NATIVE(to_index)(const struct AMA_NATIVE(State) *, size_t *);
/** Construct exactly 0..SIZE_MAX directly, without decimal/default conversion. */
AmaStatus AMA_NATIVE(from_size)(struct AMA_NATIVE(State) *, size_t);
/** Construct canonical big-endian sign/magnitude directly in fresh storage.
 * Empty/leading-zero magnitudes and negative zero fail without a live payload.
 * The bounded input borrow ends on return and is never retained.
 */
AmaStatus AMA_NATIVE(from_magnitude)(struct AMA_NATIVE(State) *, bool, AmaByteView);
/** Compare two live mathematical integers, publishing -1/0/1 only on success. */
AmaStatus AMA_NATIVE(compare)(const struct AMA_NATIVE(State) *, const struct AMA_NATIVE(State) *, int *);
/** Prepare an arbitrary-precision signed sum before replacing the live result.
 * Sources may alias the destination; preparation failure preserves all values.
 */
AmaStatus AMA_NATIVE(add)(struct AMA_NATIVE(State) *, const struct AMA_NATIVE(State) *, const struct AMA_NATIVE(State) *);
#endif
