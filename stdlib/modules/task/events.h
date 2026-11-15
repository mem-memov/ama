/** @file
 * @brief Own a native completion queue independently of Ama callback state.
 * [ASYNC-05](../../../spec/language/async-and-parallelism.md#async-05)
 * requires native-only callback resources and executor-only imports.
 */
#include "native/events/event.h"
struct AMA_NATIVE(State) { AmaNativeEventService *service; };
