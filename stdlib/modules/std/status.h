/**
 * @file
 * @brief Own a retained immutable native status view with success default and value cloning in the native State layout.
 *
 * All inputs/views are operation-scoped; generated interfaces establish the native naming context. See the failure/resource library contract for the explicit standard-root binding.
 * @see [Fixture and coverage contract](../../../docs/implementation/library-tests-and-build.md)
 * [ERROR-06](../../../spec/implementation/diagnostics.md#error-06)
 * preserves the last-refusal snapshot as read-only ordinary value data.
 */
#ifndef AMA_STDLIB_STATUS_PAYLOAD_H
#define AMA_STDLIB_STATUS_PAYLOAD_H
#include "native/status/status.h"
/**
 * Module-qualified native payload only; generated interface supplies AMA_NATIVE and required SDK
 * declarations. No persistent Ama identity may be hidden in this State.
 */
struct AMA_NATIVE(State) { AmaStatus value; };
#endif
