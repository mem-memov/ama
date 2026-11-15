/** @file
 * @brief Submit one native-only endpoint to an independently resident backend, never an Ama view or handle.
 */
#ifndef AMA_ASYNC_TEST_BACKEND_NATIVE_H
#define AMA_ASYNC_TEST_BACKEND_NATIVE_H
#include "backend.h"
#include "native/events/event.h"
/** Capture an independent endpoint and binary bytes before returning; immediate mode completes synchronously. */
AmaStatus backend_submit(const AmaNativeEventSubmission *, bool immediate);
#endif
