/** @file
 * @brief Exercise canonical scalar boundaries independently of the encoder/decoder agreeing with each other.
 * Tables distinguish typed UTF-8 from binary bytes and canonical integer data from raw magnitudes.
 */
/** Every ASCII byte is a scalar; isolated continuation/lead bytes are never complete scalars. */
static void it_validates_utf8_boundaries_without_locale_or_terminators(void) {
    for (unsigned value = 0; value <= 255; value++) {
        unsigned char byte = (unsigned char)value;
        assert(ama_message_utf8((AmaByteView){&byte, 1}) == (value <= 127));
    }
    static const unsigned char valid[][4] = {
        {0xc2, 0x80}, {0xdf, 0xbf}, {0xe0, 0xa0, 0x80}, {0xed, 0x9f, 0xbf},
        {0xee, 0x80, 0x80}, {0xef, 0xbf, 0xbf}, {0xf0, 0x90, 0x80, 0x80}, {0xf4, 0x8f, 0xbf, 0xbf}
    };
    static const size_t lengths[] = {2, 2, 3, 3, 3, 3, 4, 4};
    for (size_t index = 0; index < sizeof(lengths) / sizeof(*lengths); index++) {
        assert(ama_message_utf8((AmaByteView){valid[index], lengths[index]}));
        for (size_t prefix = 1; prefix < lengths[index]; prefix++) {
            assert(!ama_message_utf8((AmaByteView){valid[index], prefix}));
        }
        unsigned char invalid[4];
        memcpy(invalid, valid[index], sizeof(invalid));
        invalid[lengths[index] - 1] = 0x7f;
        assert(!ama_message_utf8((AmaByteView){invalid, lengths[index]}));
        invalid[lengths[index] - 1] = 0xc0;
        assert(!ama_message_utf8((AmaByteView){invalid, lengths[index]}));
    }
    static const unsigned char invalid[][4] = {
        {0xc0, 0x80}, {0xc1, 0xbf}, {0xe0, 0x9f, 0xbf}, {0xed, 0xa0, 0x80},
        {0xf0, 0x8f, 0xbf, 0xbf}, {0xf4, 0x90, 0x80, 0x80}, {0xf5, 0x80, 0x80, 0x80}
    };
    static const size_t bad_lengths[] = {2, 2, 3, 3, 4, 4, 4};
    for (size_t index = 0; index < sizeof(bad_lengths) / sizeof(*bad_lengths); index++) {
        assert(!ama_message_utf8((AmaByteView){invalid[index], bad_lengths[index]}));
    }
    assert(ama_message_utf8((AmaByteView){NULL, 0}));
}

/** Canonical primitives reject negative zero, leading zeros and malformed text before allocating. */
static void it_rejects_noncanonical_values_and_enforces_exact_field_bounds(void) {
    const unsigned char zero[] = {0}, leading[] = {0, 1}, invalid_text[] = {255};
    const AmaNativeMessageField *integer = &field_schema[1];
    assert(ama_message_scalar(integer, (AmaNativeMessageValue){0, {zero, 1}}) == AMA_MESSAGE_ACCEPTED);
    assert(ama_message_scalar(integer, (AmaNativeMessageValue){1, {zero, 1}}) == AMA_MESSAGE_MALFORMED);
    assert(ama_message_scalar(integer, (AmaNativeMessageValue){0, {leading, 2}}) == AMA_MESSAGE_MALFORMED);
    assert(ama_message_scalar(integer, (AmaNativeMessageValue){0, {NULL, 0}}) == AMA_MESSAGE_MALFORMED);
    assert(ama_message_scalar(integer, (AmaNativeMessageValue){2, {magnitude, 3}}) == AMA_MESSAGE_MALFORMED);
    assert(ama_message_scalar(integer, (AmaNativeMessageValue){0, {NULL, 1}}) == AMA_MESSAGE_MALFORMED);
    assert(ama_message_scalar(integer, (AmaNativeMessageValue){0, {NULL, SIZE_MAX}}) == AMA_MESSAGE_LIMIT);
    unsigned char maximum[17];
    memset(maximum, 255, sizeof(maximum));
    assert(ama_message_scalar(integer, (AmaNativeMessageValue){0, {maximum, 16}}) == AMA_MESSAGE_ACCEPTED);
    assert(ama_message_scalar(integer, (AmaNativeMessageValue){0, {maximum, 17}}) == AMA_MESSAGE_LIMIT);
    assert(ama_message_scalar(&field_schema[2], (AmaNativeMessageValue){0, {invalid_text, 1}}) == AMA_MESSAGE_MALFORMED);
    assert(ama_message_scalar(&field_schema[3], (AmaNativeMessageValue){0, {invalid_text, 1}}) == AMA_MESSAGE_ACCEPTED);
    assert(ama_message_scalar(&field_schema[2], (AmaNativeMessageValue){1, {NULL, 0}}) == AMA_MESSAGE_MALFORMED);
    assert(ama_message_scalar(&field_schema[0], (AmaNativeMessageValue){2, {NULL, 0}}) == AMA_MESSAGE_MALFORMED);
    AmaNativeMessageFieldValue changed[4];
    memcpy(changed, fields, sizeof(changed));
    changed[3] = (AmaNativeMessageFieldValue){NULL, 0};
    AmaNativeMessageBytes wire = {0};
    accepted(ama_native_message_encode(&schema, changed, &provider, &wire));
    AmaNativeMessageDecoded decoded = {0};
    accepted(ama_native_message_decode(&schema, (AmaByteView){wire.data, wire.length}, &provider, &decoded));
    assert(decoded.fields[3].count == 0 && decoded.fields[3].items == NULL);
    ama_native_message_decoded_drop(&decoded);
    ama_native_message_bytes_drop(&wire);
    changed[3] = (AmaNativeMessageFieldValue){bytes_value, 4};
    AmaNativeMessageResult result = ama_native_message_encode(&schema, changed, &provider, &wire);
    assert(result.status.code == AMA_OK && result.reason == AMA_MESSAGE_LIMIT && wire.data == NULL);
    changed[3] = fields[3];
    changed[0].count = 0;
    result = ama_native_message_encode(&schema, changed, &provider, &wire);
    assert(result.status.code == AMA_OK && result.reason == AMA_MESSAGE_MALFORMED && wire.data == NULL);
}
