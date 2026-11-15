/** @file
 * @brief Validate before allocation and privately construct independent decoded data with prefix cleanup.
 * [PAR-03](../../../../spec/language/async-and-parallelism.md#par-03) forbids
 * publishing borrowed sender storage or a partially initialized received value.
 */
#include "../internal.h"

/** Allocate each independent native owner only after complete protocol validation.
 * Arrays contain data borrows into the privately copied envelope; freeing them
 * requires no traversal or native payload callbacks, including on partial failure.
 */
AmaNativeMessageResult ama_native_message_decode(const AmaNativeMessageSchema *schema, AmaByteView bytes,
    const AmaNativeCodeProvider *provider, AmaNativeMessageDecoded *output) {
    if (!ama_message_schema_valid(schema) || provider == NULL || output == NULL
        || output->fields != NULL || output->values != NULL || output->wire != NULL
        || output->field_count != 0 || output->code.resource != NULL || output->code.release != NULL) {
        return ama_message_failure(ama_status_make(AMA_INVALID_STATE, NULL, 0));
    }
    size_t count = 0;
    AmaNativeMessageReason reason = ama_message_scan(schema, bytes, &count, NULL);
    if (reason != AMA_MESSAGE_ACCEPTED) { return ama_message_result(reason); }
    if (schema->field_count > SIZE_MAX / sizeof(*output->fields) || count > SIZE_MAX / sizeof(*output->values)) {
        return ama_message_failure(ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0));
    }
    AmaNativeMessageDecoded candidate = {.field_count = schema->field_count};
    AmaStatus status = ama_native_code_acquire(provider, &candidate.code);
    if (status.code != AMA_OK) { return ama_message_failure(status); }
    if (candidate.field_count != 0) {
        candidate.fields = malloc(candidate.field_count * sizeof(*candidate.fields));
        if (candidate.fields == NULL) { goto allocation_failed; }
    }
    if (count != 0) {
        candidate.values = malloc(count * sizeof(*candidate.values));
        if (candidate.values == NULL) { goto allocation_failed; }
    }
    candidate.wire = malloc(bytes.length);
    if (candidate.wire == NULL) { goto allocation_failed; }
    memcpy(candidate.wire, bytes.data, bytes.length);
    reason = ama_message_scan(schema, (AmaByteView){candidate.wire, bytes.length}, &count, &candidate);
    if (reason != AMA_MESSAGE_ACCEPTED) {
        ama_native_message_decoded_drop(&candidate);
        return ama_message_failure(ama_status_make(AMA_INVALID_STATE, NULL, 0));
    }
    *output = candidate;
    return ama_message_result(AMA_MESSAGE_ACCEPTED);
allocation_failed:
    ama_native_message_decoded_drop(&candidate);
    return ama_message_failure(ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0));
}
