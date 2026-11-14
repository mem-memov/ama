/** @file
 * @brief Construct arbitrary precision directly from the declared C magnitude transport.
 * [EMBED-04](../../../../spec/implementation/c-library.md#embed-04) transports
 * mathematical magnitude without narrowing or routing binary data through literals.
 */
/** Reverse canonical big-endian octets into one fresh little-endian allocation.
 * Malformed contents grant no ownership; zero has one zero octet and no minus.
 * Buffer validity is a C borrow precondition; NULL/count consistency is checked.
 */
AmaStatus AMA_NATIVE(from_magnitude)(AMA_NATIVE(State) *destination, bool negative,
    AmaByteView magnitude) {
    *destination = (AMA_NATIVE(State)){0};
    if (magnitude.data == NULL || magnitude.length == 0
        || (magnitude.length > 1 && magnitude.data[0] == 0)
        || (negative && magnitude.length == 1 && magnitude.data[0] == 0)) {
        return ama_status_make(AMA_INVALID_LITERAL, NULL, 0);
    }
    AmaStatus status = AMA_NATIVE(prepare)(destination, magnitude.length);
    if (status.code != AMA_OK) { return status; }
    for (size_t index = 0; index < magnitude.length; index++) {
        destination->limbs[index] = magnitude.data[magnitude.length - index - 1];
    }
    destination->negative = negative;
    destination->live = true;
    return ama_status_success();
}
