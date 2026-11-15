/** @file
 * @brief Own one native channel registration; borrowed Ama operation/port inputs remain declared parameters.
 * No full view, reference-storage pointer or context is retained in this payload.
 */
#include "native/channels/channel.h"
struct AMA_NATIVE(State) { AmaNativeChannelWait wait; };
