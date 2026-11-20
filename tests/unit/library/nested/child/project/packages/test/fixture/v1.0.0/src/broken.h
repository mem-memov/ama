/** @file
 * @brief Declare a deliberately failing child constructor for partial-owner rollback tests.
 */
#ifndef AMA_NESTED_BROKEN_STATE_H
#define AMA_NESTED_BROKEN_STATE_H
#include "native/abi/services.h"
/** No foreign resource is acquired before this constructor fails. */
struct AMA_NATIVE(State) { bool unused; };
/** Fail before successful initialization; the native destructor must receive no credit. */
AmaStatus AMA_NATIVE(initialize)(struct AMA_NATIVE(State) *);
/** A fully initialized value would own no resource. */
AmaStatus AMA_NATIVE(destroy)(struct AMA_NATIVE(State) *);
#endif
