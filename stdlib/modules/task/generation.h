/** @file
 * @brief Store one declared native scalar generation without any encoded Ama identity.
 * [ASYNC-04](../../../spec/language/async-and-parallelism.md#async-04)
 * uses explicit nonwrapping values in declared epoch/generation fields.
 */
#include <stddef.h>
struct AMA_NATIVE(State) { size_t value; };
