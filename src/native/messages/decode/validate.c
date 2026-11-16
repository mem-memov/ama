/** @file
 * @brief Expose allocation-free whole-message admission independently of receiver construction.
 * [PAR-03](../../../../spec/language/async-and-parallelism.md#par-03) and
 * [PAR-04](../../../../spec/language/async-and-parallelism.md#par-04) require
 * schema/size rejection before whole-message channel acceptance.
 */
#include "../internal.h"

/** Schema errors are native configuration failures; unsupported wire is ordinary data. Component contract: [PAR-03](../../../../spec/language/async-and-parallelism.md#par-03), [PAR-04](../../../../spec/language/async-and-parallelism.md#par-04). */
AmaNativeMessageResult ama_native_message_validate(const AmaNativeMessageSchema *schema, AmaByteView bytes) {
    AmaStatus status = ama_native_message_schema_check(schema);
    if (status.code != AMA_OK) { return ama_message_failure(status); }
    size_t count = 0;
    return ama_message_result(ama_message_scan(schema, bytes, &count, NULL));
}
