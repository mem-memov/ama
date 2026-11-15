/** @file
 * @brief Hold native service lifetime for one explicitly selected method.
 * [ASYNC-05](../../../../spec/language/async-and-parallelism.md#async-05)
 * forbids retaining Ama state; this resource owns only a native service lease.
 */
#include "native/events/event.h"
struct AMA_NATIVE(State) { AmaNativeEventKeepalive lease; };
