/** @file
 * @brief Expose allocation-free whole-message admission independently of receiver construction.
 * [PAR-03](../../../../spec/language/async-and-parallelism.md#par-03) and
 * [PAR-04](../../../../spec/language/async-and-parallelism.md#par-04) require
 * schema/size rejection before whole-message channel acceptance.
 */
#include "../internal.h"

/** Schema errors are native configuration failures; unsupported wire is ordinary data. */
AmaNativeMessageResult ama_native_message_validate(const AmaNativeMessageSchema *schema, AmaByteView bytes) {
    if (!ama_message_schema_valid(schema)) { return ama_message_failure(ama_status_make(AMA_INVALID_STATE, NULL, 0)); }
    size_t count = 0;
    return ama_message_result(ama_message_scan(schema, bytes, &count, NULL));
}
