#ifndef AMA_NATIVE_ABI_SERVICES_H
#define AMA_NATIVE_ABI_SERVICES_H

#include "../status/status.h"

/* Read-only operation-scoped value snapshot, not a pointer into runtime state. */
typedef struct {
    AmaStatus caller_refusal;
} AmaNativeServices;

#endif
