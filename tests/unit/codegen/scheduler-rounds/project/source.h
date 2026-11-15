/** @file
 * @brief Own the standalone native hub without storing any Ama identity in native state.
 * [ASYNC-05](../../../../../spec/language/async-and-parallelism.md#async-05)
 * permits native resources while all task/registration relationships stay declared in Ama.
 */
#include "native/events/event.h"
struct AMA_NATIVE(State) { AmaNativeEventService *service; };
