/** @file
 * @brief Own a native worker job and service lease independently of every Ama borrow.
 * [ASYNC-05](../../../../../spec/language/async-and-parallelism.md#async-05)
 * forbids retained Ama state; the job owns only a native endpoint and scalar coordination.
 */
#include "native/events/event.h"
struct AMA_NATIVE(Job);
struct AMA_NATIVE(State) { AmaNativeEventKeepalive lease; struct AMA_NATIVE(Job) *job; };
