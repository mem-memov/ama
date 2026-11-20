/** @file
 * @brief Declare a stateless signal value whose ordinary C helper exposes nested activation lifetime.
 */
#ifndef AMA_NESTED_SIGNAL_STATE_H
#define AMA_NESTED_SIGNAL_STATE_H
#include "native/abi/services.h"
/** No callback, context or borrowed value is stored in this payload. */
struct AMA_NATIVE(State) { bool unused; };
/** Initialize no resource and execute no signal body. */
AmaStatus AMA_NATIVE(initialize)(struct AMA_NATIVE(State) *);
/** Consume no native resource. */
AmaStatus AMA_NATIVE(destroy)(struct AMA_NATIVE(State) *);
/** A stateless independent clone owns no shared destructor responsibility. */
AmaStatus AMA_NATIVE(clone)(struct AMA_NATIVE(State) *,const struct AMA_NATIVE(State) *);
#endif
