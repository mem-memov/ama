/** @file
 * @brief Prepare a bounded independently owned envelope before publishing any sender output.
 * [PAR-03](../../../../spec/language/async-and-parallelism.md#par-03) keeps
 * application data rejection distinct from technical allocation/code-credit failure.
 */
#include "../internal.h"

/** Validate shape and total budget before scalar payloads, then allocate once.
 * The sizing emitter reads metadata only, so an envelope that cannot fit never
 * scans large text/integer bytes merely to reject its complete size afterward.
 * Component contract: [PAR-03](../../../../spec/language/async-and-parallelism.md#par-03). */
AmaNativeMessageResult ama_native_message_encode(const AmaNativeMessageSchema *schema,
    const AmaNativeMessageFieldValue *fields, const AmaNativeCodeProvider *provider, AmaNativeMessageBytes *output) {
    if (ama_native_message_schema_check(schema).code != AMA_OK || provider == NULL || output == NULL
        || output->data != NULL || output->length != 0 || output->code.resource != NULL || output->code.release != NULL
        || (schema->field_count != 0 && fields == NULL)) {
        return ama_message_failure(ama_status_make(AMA_INVALID_STATE, NULL, 0));
    }
    for (size_t index = 0; index < schema->field_count; index++) {
        const AmaNativeMessageField *field = &schema->fields[index];
        if ((!field->list && fields[index].count != 1)
            || (fields[index].count != 0 && fields[index].items == NULL)) { return ama_message_result(AMA_MESSAGE_MALFORMED); }
        if (fields[index].count > field->maximum_count) { return ama_message_result(AMA_MESSAGE_LIMIT); }
    }
    AmaMessageWriter counter = {.limit = schema->maximum_wire_bytes};
    if (!ama_message_emit(schema, fields, &counter)) { return ama_message_result(AMA_MESSAGE_LIMIT); }
    for (size_t index = 0; index < schema->field_count; index++) {
        for (size_t item = 0; item < fields[index].count; item++) {
            AmaNativeMessageReason reason = ama_message_scalar(&schema->fields[index], fields[index].items[item]);
            if (reason != AMA_MESSAGE_ACCEPTED) { return ama_message_result(reason); }
        }
    }
    AmaNativeMessageBytes candidate = {0};
    AmaStatus status = ama_native_code_acquire(provider, &candidate.code);
    if (status.code != AMA_OK) { return ama_message_failure(status); }
    candidate.data = malloc(counter.position);
    if (candidate.data == NULL) {
        ama_native_message_bytes_drop(&candidate);
        return ama_message_failure(ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0));
    }
    candidate.length = counter.position;
    AmaMessageWriter writer = {candidate.data, 0, candidate.length};
    if (!ama_message_emit(schema, fields, &writer) || writer.position != candidate.length) {
        ama_native_message_bytes_drop(&candidate);
        return ama_message_failure(ama_status_make(AMA_INVALID_STATE, NULL, 0));
    }
    *output = candidate;
    return ama_message_result(AMA_MESSAGE_ACCEPTED);
}
