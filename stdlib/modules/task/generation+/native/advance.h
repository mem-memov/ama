/** @file
 * @brief Reserve a fresh scalar generation without wrap or partial publication.
 * [ASYNC-04](../../../../../spec/language/async-and-parallelism.md#async-04)
 * requires exhaustion before changing a live enrollment or registration identity.
 */
#ifndef AMA_STANDARD_TASK_GENERATION_ADVANCE_H
#define AMA_STANDARD_TASK_GENERATION_ADVANCE_H
#include <stdbool.h>
#include <stdint.h>
#include "native/abi/bytes.h"
/** Reserve the successor within an explicit inclusive bound. Rejection leaves
 * output unchanged; a caller publishes only the successfully prepared value.
 */
static inline bool AMA_NATIVE(reserve)(size_t current, size_t limit, size_t *next) {
    if (current >= limit) { return false; }
    *next = current + 1;
    return true;
}
/** Parse counted unsigned ASCII decimal, proving every prefix fits size_t before
 * multiplying. Rejection preserves output; empty text and signs are invalid.
 */
static inline bool AMA_NATIVE(parse)(AmaByteView text, size_t *output) {
    if (text.length == 0 || text.data == NULL) { return false; }
    size_t value = 0;
    for (size_t index = 0; index < text.length; index++) {
        unsigned char byte = text.data[index];
        if (byte < '0' || byte > '9') { return false; }
        size_t digit = byte - '0';
        if (value > (SIZE_MAX - digit) / 10) { return false; }
        value = value * 10 + digit;
    }
    *output = value;
    return true;
}
#endif
