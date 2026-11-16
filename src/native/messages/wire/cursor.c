/** @file
 * @brief Read and write exact bounded wire slices with fixed network-order counts.
 * [PAR-03](../../../../spec/language/async-and-parallelism.md#par-03) requires
 * size validation before touching data or publishing a received value.
 */
#include "../internal.h"
_Static_assert(sizeof(size_t) <= sizeof(uint64_t), "Message counts need a representable wire domain");

/** Bound subtraction precedes addition; failure leaves both position and output unchanged. Component contract: [PAR-03](../../../../spec/language/async-and-parallelism.md#par-03). */
bool ama_message_read(AmaMessageReader *reader, size_t count, AmaByteView *output) {
    if (count > reader->bytes.length - reader->position || (count != 0 && reader->bytes.data == NULL)) { return false; }
    *output = (AmaByteView){reader->bytes.data == NULL ? NULL : reader->bytes.data + reader->position, count};
    reader->position += count;
    return true;
}

/** Stage the cursor so unrepresentable counts cannot consume input. Component contract: [PAR-03](../../../../spec/language/async-and-parallelism.md#par-03). */
bool ama_message_read_size(AmaMessageReader *reader, size_t *output) {
    AmaMessageReader staged = *reader;
    AmaByteView bytes;
    if (!ama_message_read(&staged, 8, &bytes)) { return false; }
    uint64_t value = 0;
    for (size_t index = 0; index < 8; index++) { value = (value << 8) | bytes.data[index]; }
    if (value > SIZE_MAX) { return false; }
    *output = (size_t)value;
    *reader = staged;
    return true;
}

/** The null destination performs sizing only; all nonempty source slices remain valid borrows. Component contract: [PAR-03](../../../../spec/language/async-and-parallelism.md#par-03). */
bool ama_message_write(AmaMessageWriter *writer, AmaByteView bytes) {
    if (bytes.length > writer->limit - writer->position) { return false; }
    if (writer->data != NULL && bytes.length != 0) {
        memcpy(writer->data + writer->position, bytes.data, bytes.length);
    }
    writer->position += bytes.length;
    return true;
}

/** Fixed width avoids alternative encodings and machine-endian dependence. Component contract: [PAR-03](../../../../spec/language/async-and-parallelism.md#par-03). */
bool ama_message_write_size(AmaMessageWriter *writer, size_t count) {
    unsigned char bytes[8];
    uint64_t value = count;
    for (size_t index = 8; index != 0;) { bytes[--index] = (unsigned char)value; value >>= 8; }
    return ama_message_write(writer, (AmaByteView){bytes, sizeof(bytes)});
}
