/** @file
 * @brief Validate canonical primitive message data without reading oversized or absent slices.
 * [PAR-03](../../../../spec/language/async-and-parallelism.md#par-03) requires
 * validation before constructing independently owned receiver values.
 */
#include "../internal.h"

/** Canonical sign/magnitude has exactly one zero byte for zero, never a negative zero. */
AmaNativeMessageReason ama_message_scalar(const AmaNativeMessageField *field, AmaNativeMessageValue value) {
    if (value.bytes.length > field->maximum_bytes) { return AMA_MESSAGE_LIMIT; }
    if ((value.bytes.data == NULL && value.bytes.length != 0) || value.flag > 1) { return AMA_MESSAGE_MALFORMED; }
    if (field->kind == AMA_MESSAGE_CONDITION) {
        return value.bytes.length == 0 ? AMA_MESSAGE_ACCEPTED : AMA_MESSAGE_MALFORMED;
    }
    if (field->kind == AMA_MESSAGE_INTEGER) {
        if (value.bytes.length == 0 || (value.bytes.data[0] == 0 && (value.flag != 0 || value.bytes.length != 1))) {
            return AMA_MESSAGE_MALFORMED;
        }
    } else if (value.flag != 0 || (field->kind == AMA_MESSAGE_TEXT && !ama_message_utf8(value.bytes))) {
        return AMA_MESSAGE_MALFORMED;
    }
    return AMA_MESSAGE_ACCEPTED;
}
