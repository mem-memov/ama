/** @file
 * @brief Own one binary value while allowing independent clones to share immutable bytes.
 * [COPY-01](../../spec/language/values-and-references.md#copy-01) permits backing
 * sharing without merging Ama identities; [PROTO-06](../../spec/implementation/native-interface.md#proto-06)
 * requires each successful clone to acquire and eventually release its own share.
 */
#ifndef AMA_STANDARD_BYTES_STATE_H
#define AMA_STANDARD_BYTES_STATE_H
#include "native/abi/bytes.h"
#include "native/abi/services.h"
struct AMA_NATIVE(Backing);
/** Own one backing share, or a live allocation-free empty value. No Ama handle,
 * borrowed byte view or asynchronous callback is retained in native storage.
 */
struct AMA_NATIVE(State) { struct AMA_NATIVE(Backing) *backing; bool live; };
/** Construct allocation-free empty bytes in fresh native storage. */
AmaStatus AMA_NATIVE(initialize)(struct AMA_NATIVE(State) *);
/** Consume exactly this live value's share, freeing the final immutable backing. */
AmaStatus AMA_NATIVE(destroy)(struct AMA_NATIVE(State) *);
/** Copy counted binary and copy bytes into fresh immutable backing. */
AmaStatus AMA_NATIVE(from_literal)(struct AMA_NATIVE(State) *, AmaByteView);
/** Acquire a representable share before publishing an independent fresh value. */
AmaStatus AMA_NATIVE(clone)(struct AMA_NATIVE(State) *, const struct AMA_NATIVE(State) *);
/** Read nonempty truth without executing or modifying the bytes value. */
AmaStatus AMA_NATIVE(to_condition)(const struct AMA_NATIVE(State) *, bool *);
/** Borrow immutable bytes until their owning value is replaced/finalized. For
 * an Ama-borrowed state, the view must also end before the enclosing native
 * activation returns; it cannot be retained across an Ama execution boundary.
 */
AmaStatus AMA_NATIVE(view)(const struct AMA_NATIVE(State) *, AmaByteView *);
/** Prepare new immutable bytes before consuming the old live value. Source bytes
 * may alias its old backing; preparation failure preserves the complete value.
 */
AmaStatus AMA_NATIVE(replace)(struct AMA_NATIVE(State) *, AmaByteView);
#endif
