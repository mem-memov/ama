/** @file
 * @brief Verify real native message data ownership, malformed envelopes and exact failure cleanup.
 * Scenarios compose schema validation, sizing, wire traversal and owner publication.
 */
#include "common.c"
#include "support/scalars.c"
#include "support/wire.c"

/** Decode an independent record and retain its code credit after the source envelope dies. */
static void it_round_trips_typed_data_without_borrowing_the_sender_or_schema(void) {
    AmaNativeMessageBytes wire = {0};
    accepted(ama_native_message_encode(&schema, fields, &provider, &wire));
    assert(code.credits == 1 && live_allocations == 1);
    AmaNativeMessageDecoded output = {0};
    accepted(ama_native_message_decode(&schema, (AmaByteView){wire.data, wire.length}, &provider, &output));
    assert(code.credits == 2 && live_allocations == 4 && output.field_count == 4);
    assert(output.wire != wire.data && output.fields[0].items[0].flag == 1);
    assert(output.fields[1].items[0].flag == 1 && output.fields[1].items[0].bytes.length == 3);
    assert(memcmp(output.fields[1].items[0].bytes.data, magnitude, sizeof(magnitude)) == 0);
    assert(output.fields[2].items[0].bytes.length == sizeof(text));
    assert(memcmp(output.fields[2].items[0].bytes.data, text, sizeof(text)) == 0);
    assert(output.fields[3].count == 2 && output.fields[3].items[1].bytes.length == 0);
    ama_native_message_bytes_drop(&wire);
    assert(code.credits == 1 && live_allocations == 3);
    assert(memcmp(output.fields[3].items[0].bytes.data, binary, sizeof(binary)) == 0);
    ama_native_message_decoded_drop(&output);
    ama_native_message_decoded_drop(&output);
    ama_native_message_bytes_drop(&wire);
    assert(code.credits == 0 && live_allocations == 0);
}

/** Every truncated prefix and trailing byte fails before allocation or code acquisition. */
static void it_rejects_truncation_trailing_data_versions_and_declared_limits(void) {
    AmaNativeMessageBytes wire = {0};
    accepted(ama_native_message_encode(&schema, fields, &provider, &wire));
    size_t before = allocation_count;
    for (size_t length = 0; length < wire.length; length++) {
        AmaNativeMessageDecoded output = {0};
        rejected(ama_native_message_decode(&schema, (AmaByteView){wire.data, length}, &provider, &output),
            AMA_MESSAGE_MALFORMED, &output);
    }
    unsigned char extended[1024];
    memcpy(extended, wire.data, wire.length);
    extended[wire.length] = 0;
    AmaNativeMessageDecoded output = {0};
    rejected(ama_native_message_decode(&schema, (AmaByteView){extended, wire.length + 1}, &provider, &output),
        AMA_MESSAGE_MALFORMED, &output);
    extended[0] = 'X';
    rejected(ama_native_message_decode(&schema, (AmaByteView){extended, wire.length}, &provider, &output),
        AMA_MESSAGE_UNSUPPORTED_FORMAT, &output);
    AmaNativeMessageSchema wrong = schema;
    wrong.version++;
    rejected(ama_native_message_decode(&wrong, (AmaByteView){wire.data, wire.length}, &provider, &output),
        AMA_MESSAGE_SCHEMA_MISMATCH, &output);
    wrong = schema;
    wrong.maximum_wire_bytes = wire.length - 1;
    rejected(ama_native_message_decode(&wrong, (AmaByteView){wire.data, wire.length}, &provider, &output),
        AMA_MESSAGE_LIMIT, &output);
    assert(allocation_count == before && code.credits == 1);
    ama_native_message_bytes_drop(&wire);
}

/** Fail each allocation prefix in both directions, including independent native code admission. */
static void it_cleans_every_allocation_prefix_and_preserves_output_on_technical_failure(void) {
    AmaNativeMessageBytes wire = {0};
    fail_at = allocation_count + 1;
    AmaNativeMessageResult result = ama_native_message_encode(&schema, fields, &provider, &wire);
    assert(result.status.code == AMA_OUT_OF_MEMORY && wire.data == NULL && code.credits == 0 && live_allocations == 0);
    fail_at = 0;
    accepted(ama_native_message_encode(&schema, fields, &provider, &wire));
    for (size_t failure = 1; failure <= 3; failure++) {
        AmaNativeMessageDecoded output = {0};
        fail_at = allocation_count + failure;
        result = ama_native_message_decode(&schema, (AmaByteView){wire.data, wire.length}, &provider, &output);
        assert(result.status.code == AMA_OUT_OF_MEMORY && output.fields == NULL && output.wire == NULL);
        assert(output.values == NULL && output.code.resource == NULL && output.field_count == 0);
        assert(code.credits == 1 && live_allocations == 1);
        fail_at = 0;
    }
    code.refuse = true;
    AmaNativeMessageDecoded output = {0};
    size_t before = allocation_count;
    result = ama_native_message_decode(&schema, (AmaByteView){wire.data, wire.length}, &provider, &output);
    assert(result.status.code == AMA_LIMIT_EXCEEDED && output.wire == NULL && allocation_count == before);
    code.refuse = false;
    ama_native_message_bytes_drop(&wire);
    assert(code.credits == 0 && live_allocations == 0);
}

/** Run actual data and ownership handoffs; additional scalar/wire boundaries live in support cases. */
void run(void) {
    it_validates_utf8_boundaries_without_locale_or_terminators();
    it_rejects_noncanonical_values_and_enforces_exact_field_bounds();
    it_matches_fixed_wire_vectors_and_rejects_shape_substitution();
    it_preserves_cursor_outputs_when_fixed_width_reads_or_writes_do_not_fit();
    it_rejects_invalid_configuration_and_nonempty_outputs_without_effects();
    it_round_trips_typed_data_without_borrowing_the_sender_or_schema();
    it_rejects_truncation_trailing_data_versions_and_declared_limits();
    it_cleans_every_allocation_prefix_and_preserves_output_on_technical_failure();
}

/** Success requires all native allocation and code credits to retire. */
int main(void) {
    run();
    assert(live_allocations == 0 && code.credits == 0);
    return EXIT_SUCCESS;
}
