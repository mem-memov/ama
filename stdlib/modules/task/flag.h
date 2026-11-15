/** @file
 * @brief Represent one declared mutable Boolean protocol field.
 * [ASYNC-04](../../../spec/language/async-and-parallelism.md#async-04)
 * keeps membership and waiting flags in ordinary owned Ama values.
 */
#include <stdbool.h>
struct AMA_NATIVE(State) { bool value; };
