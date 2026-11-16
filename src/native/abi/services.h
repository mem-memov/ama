/**
 * @file
 * @brief Expose read-only operation-scoped ABI service values without Ama runtime pointers.
 *
 * [CABI-03](../../../spec/implementation/native-interface.md#cabi-03)
 * Copy the caller refusal snapshot as data; no mutable Ama state or execution capability is exposed.
 */
#ifndef AMA_NATIVE_ABI_SERVICES_H
#define AMA_NATIVE_ABI_SERVICES_H

#include "../status/status.h"
#include "code.h"

/* Read-only operation-scoped value snapshot, not a pointer into runtime state. */
/** Read-only per-operation service snapshot, not a retained pointer into runtime state. Message bytes retain their separate program-lifetime contract. Component contract: [CABI-03](../../../spec/implementation/native-interface.md#cabi-03). */
typedef struct {
    AmaStatus caller_refusal;
    /** Native-only image retention; this value grants no Ama access or execution. Component contract: [CABI-03](../../../spec/implementation/native-interface.md#cabi-03). */
    AmaNativeCodeProvider code;
} AmaNativeServices;

#endif
