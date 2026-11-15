/** @file
 * @brief Store immutable native event identity in a declared Ama scalar.
 * [ASYNC-05](../../../spec/language/async-and-parallelism.md#async-05)
 * requires the complete token independently of native locator reuse.
 */
#include "native/events/event.h"
struct AMA_NATIVE(State) { AmaNativeEventToken token; };
