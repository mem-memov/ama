/** @file
 * @brief Assemble the native event resource component once for an explicit native provider.
 * [ASYNC-05](../../../spec/language/async-and-parallelism.md#async-05)
 * permits native queues; this assembly imports no Ama runtime or dispatch implementation.
 */
#ifndef AMA_NATIVE_EVENTS_ASSEMBLY_H
#define AMA_NATIVE_EVENTS_ASSEMBLY_H
#include "native/events/identity/sequence.c"
#include "native/events/identity/source.c"
#include "native/events/service/gate.c"
#include "native/events/wake/resource.c"
#include "native/events/wake/signal.c"
#include "native/events/wake/wait.c"
#include "native/events/wake/poll.c"
#include "native/events/service/create.c"
#include "native/events/service/lifetime.c"
#include "native/events/submission/cell.c"
#include "native/events/submission/prepare.c"
#include "native/events/submission/arm.c"
#include "native/events/submission/detach.c"
#include "native/events/buffer/create.c"
#include "native/events/buffer/release.c"
#include "native/events/callback/port.c"
#include "native/events/callback/complete.c"
#include "native/events/queue/take.c"
#endif
