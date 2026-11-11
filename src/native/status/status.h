#ifndef AMA_NATIVE_STATUS_H
#define AMA_NATIVE_STATUS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "codes.h"

/* Message storage is immutable UTF-8 with program lifetime, never owned here.
 * NULL requires length zero. No terminator is required; code controls behavior.
 */
typedef struct {
    uint32_t code;
    const char *message;
    size_t message_length;
} AmaStatus;

static inline AmaStatus ama_status_success(void) {
    return (AmaStatus) {AMA_OK, NULL, 0};
}

static inline AmaStatus ama_status_make(uint32_t code, const char *message, size_t length) {
    return (AmaStatus) {code, message, length};
}

static inline bool ama_status_is_success(AmaStatus status) {
    return status.code == AMA_OK;
}

static inline bool ama_status_has_valid_message_view(AmaStatus status) {
    return status.message != NULL || status.message_length == 0;
}

#endif
