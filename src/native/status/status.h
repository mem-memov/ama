/**
 * @file
 * @brief Represent allocation-free native outcomes as a code and borrowed message.
 *
 * [ERROR-03](../../../spec/implementation/diagnostics.md#error-03) makes the
 * numeric code authoritative and permits an unterminated optional message.
 * [ERROR-04](../../../spec/implementation/diagnostics.md#error-04) requires
 * immutable program-lifetime message storage: these helpers never allocate,
 * retain ownership or release the borrowed bytes.
 */
#ifndef AMA_NATIVE_STATUS_H
#define AMA_NATIVE_STATUS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "codes.h"

/** Native outcome with immutable borrowed UTF-8 bytes; NULL requires zero length.
 * The caller supplies valid program-lifetime storage and an operation-specific
 * numeric code. No terminating NUL is required and wording never controls flow.
 * Component contract: [ERROR-03](../../../spec/implementation/diagnostics.md#error-03), [ERROR-04](../../../spec/implementation/diagnostics.md#error-04). */
typedef struct {
    uint32_t code;
    const char *message;
    size_t message_length;
} AmaStatus;

/** Return success with an empty message and no allocation or side effects. Component contract: [ERROR-03](../../../spec/implementation/diagnostics.md#error-03), [ERROR-04](../../../spec/implementation/diagnostics.md#error-04). */
static inline AmaStatus ama_status_success(void) {
    return (AmaStatus) {AMA_OK, NULL, 0};
}

/** Borrow the supplied immutable program-lifetime bytes without validation.
 * The caller supplies a valid message view; no ownership transfer occurs.
 * Component contract: [ERROR-03](../../../spec/implementation/diagnostics.md#error-03), [ERROR-04](../../../spec/implementation/diagnostics.md#error-04). */
static inline AmaStatus ama_status_make(uint32_t code, const char *message, size_t length) {
    return (AmaStatus) {code, message, length};
}

/** Classify success by numeric code, independently of translated message text. Component contract: [ERROR-03](../../../spec/implementation/diagnostics.md#error-03), [ERROR-04](../../../spec/implementation/diagnostics.md#error-04). */
static inline bool ama_status_is_success(AmaStatus status) {
    return status.code == AMA_OK;
}

/** Check NULL/length consistency; storage lifetime and UTF-8 remain caller duties. Component contract: [ERROR-03](../../../spec/implementation/diagnostics.md#error-03), [ERROR-04](../../../spec/implementation/diagnostics.md#error-04). */
static inline bool ama_status_has_valid_message_view(AmaStatus status) {
    return status.message != NULL || status.message_length == 0;
}

/** Recognize allocated core reasons or a nonzero provider/reason pair.
 * Core values above AMA_IO_FAILURE remain reserved. This checks encoding, not
 * whether the compiler registered a particular provider or its local reason.
 * Component contract: [ERROR-03](../../../spec/implementation/diagnostics.md#error-03), [ERROR-04](../../../spec/implementation/diagnostics.md#error-04). */
static inline bool ama_status_has_valid_code(uint32_t code) {
    return code <= AMA_IO_FAILURE || ((code >> 16) != 0 && (code & UINT32_C(65535)) != 0);
}

/** Replace malformed transport with allocation-free technical failure before use.
 * Valid status fields and message identity remain unchanged. Message storage,
 * UTF-8 and immutable program lifetime remain the native author's C obligations;
 * this helper never dereferences a message or infers validity from its wording.
 * Component contract: [ERROR-03](../../../spec/implementation/diagnostics.md#error-03), [ERROR-04](../../../spec/implementation/diagnostics.md#error-04). */
static inline AmaStatus ama_status_checked(AmaStatus status) {
    if (ama_status_has_valid_message_view(status) && ama_status_has_valid_code(status.code)) {
        return status;
    }
    static const char message[] = "Malformed native status";
    return ama_status_make(AMA_INVALID_STATE, message, sizeof(message) - 1);
}

#endif
