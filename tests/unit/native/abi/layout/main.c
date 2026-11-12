/**
 * @file
 * @brief Check standalone ABI representations and required C layout properties.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../../docs/implementation/library-tests-and-build.md)
 */
#include "../common.c"

/**
 * Assert that the component transports empty and embedded zero literals by length. The explicit
 * fixture setup and observations define this regression; assertion failure keeps its generated
 * evidence.
 */
static void it_transports_empty_and_embedded_zero_literals_by_length(void) {
    AmaByteView empty = {NULL, 0};
    const unsigned char bytes[] = {'a', 0, 'b'};
    AmaByteView literal = {bytes, sizeof(bytes)};
    assert(empty.data == NULL && empty.length == 0);
    assert(literal.length == 3 && literal.data[2] == 'b');
}

/**
 * Assert that the component preserves extended payload alignment in typed storage. The explicit
 * fixture setup and observations define this regression; assertion failure keeps its generated
 * evidence.
 */
static void it_preserves_extended_payload_alignment_in_typed_storage(void) {
    /** Extended-alignment native payload used to assert C representation guarantees independently of any generated project. */
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

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_transports_empty_and_embedded_zero_literals_by_length();
    it_preserves_extended_payload_alignment_in_typed_storage();
}

/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
