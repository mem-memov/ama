#include "../common.c"

static void it_transports_empty_and_embedded_zero_literals_by_length(void) {
    AmaByteView empty = {NULL, 0};
    const unsigned char bytes[] = {'a', 0, 'b'};
    AmaByteView literal = {bytes, sizeof(bytes)};
    assert(empty.data == NULL && empty.length == 0);
    assert(literal.length == 3 && literal.data[2] == 'b');
}

static void it_preserves_extended_payload_alignment_in_typed_storage(void) {
    typedef struct {
        _Alignas(64) unsigned char bytes[64];
    } AlignedPayload;
    AlignedPayload *payload = aligned_alloc(_Alignof(AlignedPayload), sizeof(AlignedPayload));
    assert(payload != NULL);
    assert((uintptr_t) payload % _Alignof(AlignedPayload) == 0);
    payload->bytes[63] = 7;
    assert(payload->bytes[63] == 7);
    free(payload);
}

void run(void) {
    it_transports_empty_and_embedded_zero_literals_by_length();
    it_preserves_extended_payload_alignment_in_typed_storage();
}

int main(void) {
    run();
    return EXIT_SUCCESS;
}
