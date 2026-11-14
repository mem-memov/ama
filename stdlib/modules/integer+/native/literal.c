/**
 * @file
 * @brief Decode signed decimal literals into arbitrary-precision integer magnitudes.
 * [PROTO-03](../../../../spec/implementation/native-interface.md#proto-03)
 * defines fresh construction, full host-size roundtrips and checked conversions.
 */

/** Decode exact bounded ASCII digits without a host-size range restriction.
 * N decimal digits require at most N base-256 limbs because 10^N < 256^N.
 * Each multiply/add uses at most 2559, so ordinary unsigned arithmetic is exact.
 * Failure releases all private storage and never publishes a live destination.
 */
AmaStatus AMA_NATIVE(from_literal)(AMA_NATIVE(State) *destination, AmaByteView bytes) {
    AMA_INTEGER_OBSERVE_LITERAL();
    *destination = (AMA_NATIVE(State)){0};
    if (bytes.length == 0 || bytes.data == NULL) { return ama_status_make(AMA_INVALID_LITERAL, NULL, 0); }
    if (bytes.length > PTRDIFF_MAX) { return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0); }
    bool negative = bytes.data[0] == '-';
    size_t start = negative ? 1 : 0;
    if (start == bytes.length) { return ama_status_make(AMA_INVALID_LITERAL, NULL, 0); }
    size_t count = bytes.length - start;
    AmaStatus status = AMA_NATIVE(prepare)(destination, count);
    if (status.code != AMA_OK) { return status; }
    for (size_t digit = start; digit < bytes.length; digit++) {
        if (bytes.data[digit] < '0' || bytes.data[digit] > '9') {
            status = ama_status_make(AMA_INVALID_LITERAL, NULL, 0);
            break;
        }
        unsigned carry = bytes.data[digit] - '0';
        for (size_t index = 0; index < count; index++) {
            unsigned total = (unsigned)destination->limbs[index] * 10 + carry;
            destination->limbs[index] = (unsigned char)(total % 256);
            carry = total / 256;
        }
        if (carry != 0) { status = ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0); break; }
    }
    if (status.code != AMA_OK) {
        AMA_INTEGER_RELEASE(destination->limbs);
        *destination = (AMA_NATIVE(State)){0};
        return status;
    }
    destination->negative = negative;
    AMA_NATIVE(normalize)(destination);
    destination->live = true;
    return status;
}
