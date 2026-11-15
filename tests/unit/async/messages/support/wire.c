/** @file
 * @brief Check independent wire vectors and invalid field shapes without relying on a round trip alone.
 * Explicit byte offsets are the documented fixed network-order profile, not inspected generated output.
 */
/** Empty records have one exact envelope; a condition record has one exact type/shape/value encoding. */
static void it_matches_fixed_wire_vectors_and_rejects_shape_substitution(void) {
    const unsigned char identity[] = {'x'};
    AmaNativeMessageSchema empty = {{identity, 1}, 1, NULL, 0, 29};
    unsigned char golden[29] = {'A', 'm', 'M', 1};
    golden[11] = 1;
    golden[19] = 1;
    golden[20] = 'x';
    AmaNativeMessageBytes wire = {0};
    accepted(ama_native_message_encode(&empty, NULL, &provider, &wire));
    assert(wire.length == sizeof(golden) && memcmp(wire.data, golden, sizeof(golden)) == 0);
    AmaNativeMessageDecoded output = {0};
    accepted(ama_native_message_decode(&empty, (AmaByteView){golden, sizeof(golden)}, &provider, &output));
    assert(output.field_count == 0 && output.fields == NULL && output.values == NULL && output.wire != golden);
    ama_native_message_decoded_drop(&output);
    ama_native_message_bytes_drop(&wire);
    empty.maximum_wire_bytes--;
    AmaNativeMessageResult result = ama_native_message_encode(&empty, NULL, &provider, &wire);
    assert(result.status.code == AMA_OK && result.reason == AMA_MESSAGE_LIMIT && wire.data == NULL);

    AmaNativeMessageSchema condition = {{identity, 1}, 1, field_schema, 1, 48};
    unsigned char record[48] = {'A', 'm', 'M', 1};
    memcpy(record, golden, sizeof(golden));
    record[28] = 1;
    record[29] = AMA_MESSAGE_CONDITION;
    record[38] = 1;
    record[39] = 1;
    accepted(ama_native_message_encode(&condition, fields, &provider, &wire));
    assert(wire.length == sizeof(record) && memcmp(record, wire.data, sizeof(record)) == 0);
    const size_t offsets[] = {20, 28, 29, 30, 38, 39};
    const unsigned char replacements[] = {'y', 2, 99, 2, 0, 2};
    const AmaNativeMessageReason reasons[] = {AMA_MESSAGE_SCHEMA_MISMATCH, AMA_MESSAGE_SCHEMA_MISMATCH,
        AMA_MESSAGE_SCHEMA_MISMATCH, AMA_MESSAGE_SCHEMA_MISMATCH, AMA_MESSAGE_MALFORMED, AMA_MESSAGE_MALFORMED};
    size_t before = allocation_count;
    for (size_t index = 0; index < sizeof(offsets) / sizeof(*offsets); index++) {
        unsigned char changed[48];
        memcpy(changed, record, sizeof(changed));
        changed[offsets[index]] = replacements[index];
        rejected(ama_native_message_decode(&condition, (AmaByteView){changed, sizeof(changed)}, &provider, &output),
            reasons[index], &output);
    }
    assert(allocation_count == before);
    /* This independent one-condition grammar has exactly two accepted byte strings.
     * Exhaust every one-byte substitution against that complete finite oracle. */
    for (size_t position = 0; position < sizeof(record); position++) {
        for (unsigned byte = 0; byte < 256; byte++) {
            unsigned char changed[48];
            memcpy(changed, record, sizeof(changed));
            changed[position] = (unsigned char)byte;
            AmaNativeMessageResult inspected = ama_native_message_validate(&condition,
                (AmaByteView){changed, sizeof(changed)});
            assert(inspected.status.code == AMA_OK);
            bool valid = byte == record[position] || (position == 39 && byte <= 1);
            assert((inspected.reason == AMA_MESSAGE_ACCEPTED) == valid);
        }
    }
    assert(allocation_count == before);
    ama_native_message_bytes_drop(&wire);
}

/** Bounded cursor primitives preserve state on failure, including fixed-width truncation. */
static void it_preserves_cursor_outputs_when_fixed_width_reads_or_writes_do_not_fit(void) {
    unsigned char bytes[8];
    AmaMessageWriter writer = {bytes, 0, sizeof(bytes)};
    bool written = ama_message_write_size(&writer, SIZE_MAX);
    assert(written && writer.position == 8);
    AmaMessageReader reader = {{bytes, sizeof(bytes)}, 0};
    size_t value = 91;
    bool read = ama_message_read_size(&reader, &value);
    assert(read && value == SIZE_MAX && reader.position == 8);
    for (size_t prefix = 0; prefix < 8; prefix++) {
        reader = (AmaMessageReader){{bytes, prefix}, 0};
        value = 91;
        read = ama_message_read_size(&reader, &value);
        assert(!read && value == 91 && reader.position == 0);
    }
    memset(bytes, 0xa5, sizeof(bytes));
    writer = (AmaMessageWriter){bytes, 0, 7};
    written = ama_message_write_size(&writer, 1);
    assert(!written && writer.position == 0);
    for (size_t index = 0; index < sizeof(bytes); index++) { assert(bytes[index] == 0xa5); }
    reader = (AmaMessageReader){{NULL, 0}, 0};
    AmaByteView empty = {bytes, 1};
    read = ama_message_read(&reader, 0, &empty);
    assert(read && empty.data == NULL && empty.length == 0 && reader.position == 0);
}

/** Rejected C owner/provider/schema shapes never overwrite an existing independent owner. */
static void it_rejects_invalid_configuration_and_nonempty_outputs_without_effects(void) {
    AmaNativeMessageBytes wire = {0};
    AmaNativeMessageSchema invalid = schema;
    invalid.version = 0;
    AmaNativeMessageResult result = ama_native_message_encode(&invalid, fields, &provider, &wire);
    assert(result.status.code == AMA_INVALID_STATE && wire.data == NULL);
    invalid = schema;
    invalid.fields = NULL;
    result = ama_native_message_encode(&invalid, fields, &provider, &wire);
    assert(result.status.code == AMA_INVALID_STATE && wire.data == NULL);
    AmaNativeMessageField wrong = {AMA_MESSAGE_BYTES, false, 2, 1};
    invalid = schema;
    invalid.fields = &wrong;
    invalid.field_count = 1;
    result = ama_native_message_encode(&invalid, fields, &provider, &wire);
    assert(result.status.code == AMA_INVALID_STATE && wire.data == NULL);
    AmaNativeCodeProvider malformed = {NULL, retain_code};
    result = ama_native_message_encode(&schema, fields, &malformed, &wire);
    assert(result.status.code == AMA_INVALID_STATE && wire.data == NULL);
    const AmaNativeCodeProvider resident = {0};
    accepted(ama_native_message_encode(&schema, fields, &resident, &wire));
    assert(code.credits == 0);
    unsigned char *original = wire.data;
    size_t before = allocation_count;
    result = ama_native_message_encode(&schema, fields, &provider, &wire);
    assert(result.status.code == AMA_INVALID_STATE && wire.data == original && allocation_count == before);
    AmaNativeMessageDecoded output = {0};
    accepted(ama_native_message_decode(&schema, (AmaByteView){wire.data, wire.length}, &provider, &output));
    AmaNativeMessageFieldValue *original_fields = output.fields;
    before = allocation_count;
    result = ama_native_message_decode(&schema, (AmaByteView){wire.data, wire.length}, &provider, &output);
    assert(result.status.code == AMA_INVALID_STATE && output.fields == original_fields && allocation_count == before);
    ama_native_message_decoded_drop(&output);
    ama_native_message_bytes_drop(&wire);
    ama_native_message_decoded_drop(NULL);
    ama_native_message_bytes_drop(NULL);
}
