/** @file
 * @brief Own one isolated foreign component resource without retaining any Ama borrow.
 * [EMBED-07](../../../../../../../../../../spec/implementation/c-library.md#embed-07) permits
 * an exclusively owned foreign C host resource; it does not grant parent graph access.
 */
#ifndef AMA_EMBEDDED_ADAPTER_STATE_H
#define AMA_EMBEDDED_ADAPTER_STATE_H
#include "native/abi/services.h"
/** Private heap owner of foreign context/instance controls, defined in the companion. */
struct AMA_NATIVE(Child);
/** No parent references, views or inputs escape into this payload. No clone protocol exists. */
struct AMA_NATIVE(State) { struct AMA_NATIVE(Child) *child; };
/** Create a fresh child context/root without executing a body; unwind every partial owner. */
AmaStatus AMA_NATIVE(initialize)(struct AMA_NATIVE(State) *);
/** Dispose the available child without resuming a saved position, then release the holder. */
AmaStatus AMA_NATIVE(destroy)(struct AMA_NATIVE(State) *);
#endif
