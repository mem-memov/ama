/** @file
 * @brief Own native operation rights without retaining an Ama reference in native storage.
 * [ASYNC-09](../../../spec/language/async-and-parallelism.md#async-09)
 * separates visible task lifetime from the retained native callback resource.
 */
#include "native/events/event.h"
struct AMA_NATIVE(State) { AmaNativeEventSubmission submission; };
