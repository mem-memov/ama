/** @file
 * @brief Validate a complete envelope before allocation, then populate independent native views privately.
 * [PAR-03](../../../../spec/language/async-and-parallelism.md#par-03) rejects
 * schema/version/size/canonical-data errors as ordinary protocol results.
 */
#include "../internal.h"

/** A null destination performs pure validation/counting. A nonnull destination
 * has exact arrays reserved from the first pass and reads the immutable copied
 * envelope under the same immutable schema. No field owns a separate resource.
 * Component contract: [PAR-03](../../../../spec/language/async-and-parallelism.md#par-03). */
AmaNativeMessageReason ama_message_scan(const AmaNativeMessageSchema *schema, AmaByteView bytes,
    size_t *total, AmaNativeMessageDecoded *destination) {
    if (bytes.length > schema->maximum_wire_bytes) { return AMA_MESSAGE_LIMIT; }
    if (bytes.data == NULL && bytes.length != 0) { return AMA_MESSAGE_MALFORMED; }
    if (bytes.length < 4) { return AMA_MESSAGE_MALFORMED; }
    AmaMessageReader reader = {bytes, 0};
    AmaByteView slice;
    bool read = ama_message_read(&reader, 4, &slice);
    if (!read) { return AMA_MESSAGE_MALFORMED; }
    static const unsigned char magic[] = {'A', 'm', 'M', 1};
    if (memcmp(slice.data, magic, sizeof(magic)) != 0) { return AMA_MESSAGE_UNSUPPORTED_FORMAT; }
    size_t version, length, field_count;
    if (!ama_message_read_size(&reader, &version) || !ama_message_read_size(&reader, &length)
        || !ama_message_read(&reader, length, &slice)) { return AMA_MESSAGE_MALFORMED; }
    if (version != schema->version || length != schema->name.length
        || memcmp(slice.data, schema->name.data, length) != 0) { return AMA_MESSAGE_SCHEMA_MISMATCH; }
    if (!ama_message_read_size(&reader, &field_count)) { return AMA_MESSAGE_MALFORMED; }
    if (field_count != schema->field_count) { return AMA_MESSAGE_SCHEMA_MISMATCH; }
    size_t count = 0;
    for (size_t index = 0; index < field_count; index++) {
        const AmaNativeMessageField *field = &schema->fields[index];
        size_t items;
        if (!ama_message_read(&reader, 2, &slice) || !ama_message_read_size(&reader, &items)) {
            return AMA_MESSAGE_MALFORMED;
        }
        if (slice.data[0] != field->kind || slice.data[1] != (field->list ? 1 : 0)) { return AMA_MESSAGE_SCHEMA_MISMATCH; }
        if (!field->list && items != 1) { return AMA_MESSAGE_MALFORMED; }
        if (items > field->maximum_count || items > SIZE_MAX - count) { return AMA_MESSAGE_LIMIT; }
        if (items > (reader.bytes.length - reader.position) / 9) { return AMA_MESSAGE_MALFORMED; }
        if (destination != NULL) {
            destination->fields[index] = (AmaNativeMessageFieldValue){
                items == 0 ? NULL : destination->values + count, items};
        }
        for (size_t item = 0; item < items; item++) {
            AmaByteView flag, payload;
            if (!ama_message_read(&reader, 1, &flag) || !ama_message_read_size(&reader, &length)
                || !ama_message_read(&reader, length, &payload)) { return AMA_MESSAGE_MALFORMED; }
            AmaNativeMessageValue value = {flag.data[0], payload};
            AmaNativeMessageReason reason = ama_message_scalar(field, value);
            if (reason != AMA_MESSAGE_ACCEPTED) { return reason; }
            if (destination != NULL) { destination->values[count + item] = value; }
        }
        count += items;
    }
    if (reader.position != reader.bytes.length) { return AMA_MESSAGE_MALFORMED; }
    *total = count;
    return AMA_MESSAGE_ACCEPTED;
}
