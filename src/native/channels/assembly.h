/** @file
 * @brief Compose one bounded native channel implementation and its data codec in the creator image.
 * [PAR-04](../../../spec/language/async-and-parallelism.md#par-04) keeps
 * FIFO transport independent of Ama execution, decoder publication and application acknowledgment.
 */
#ifndef AMA_NATIVE_CHANNELS_ASSEMBLY_H
#define AMA_NATIVE_CHANNELS_ASSEMBLY_H
#include "native/messages/assembly.h"
#include "service/gate.c"
#include "service/create.c"
#include "service/lifetime.c"
#include "queue/send.c"
#include "queue/take.c"
#include "endpoints/close.c"
#include "endpoints/access.c"
#include "wait/notify.c"
#include "wait/register.c"
#include "wait/cancel.c"
#include "wait/access.c"
#endif
