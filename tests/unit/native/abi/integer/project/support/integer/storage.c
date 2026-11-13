/**
 * @file
 * @brief Own fresh integer limb allocation, direct size construction, cloning and destruction.
 */

/** Allocate zeroed private magnitude storage, without selecting any native constructor.
 * A failed allocation leaves no resource and no live payload obligation.
 */
static AmaStatus AMA_NATIVE(prepare)(AMA_NATIVE(State) *destination, size_t count) {
    *destination = (AMA_NATIVE(State)){0};
    destination->limbs = AMA_INTEGER_ALLOCATE(count);
    if (destination->limbs == NULL) { return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0); }
    memset(destination->limbs, 0, count);
    destination->count = count;
    return ama_status_success();
}

/** Remove leading zero magnitude limbs and canonicalize mathematical negative zero. */
static void AMA_NATIVE(normalize)(AMA_NATIVE(State) *state) {
    while (state->count > 1 && state->limbs[state->count - 1] == 0) { state->count--; }
    if (state->count == 1 && state->limbs[0] == 0) { state->negative = false; }
}

/** Initialize native zero once, using private allocation rather than another protocol. */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *destination) {
    AMA_INTEGER_OBSERVE_DEFAULT();
    AmaStatus status = AMA_NATIVE(prepare)(destination, 1);
    if (status.code == AMA_OK) { destination->live = true; }
    return status;
}

/** Decompose the host value with unsigned quotient/remainder, never a narrowing cast.
 * Each division by 256 decreases the nonzero quotient; at most sizeof(size_t)
 * limbs suffice under the selected eight-bit-byte profile. Zero receives one limb.
 */
AmaStatus AMA_NATIVE(from_size)(AMA_NATIVE(State) *destination, size_t value) {
    AMA_INTEGER_OBSERVE_SIZE();
    size_t count = 1;
    for (size_t remaining = value; remaining >= 256; remaining /= 256) { count++; }
    AmaStatus status = AMA_NATIVE(prepare)(destination, count);
    if (status.code != AMA_OK) { return status; }
    for (size_t index = 0; index < count; index++) {
        destination->limbs[index] = (unsigned char)(value % 256);
        value /= 256;
    }
    destination->live = true;
    return status;
}

/** Copy only native mathematical data into fresh independent allocation.
 * Neither state mutation nor resource ownership crosses back to the source.
 */
AmaStatus AMA_NATIVE(clone)(AMA_NATIVE(State) *destination, const AMA_NATIVE(State) *source) {
    if (destination == source) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    *destination = (AMA_NATIVE(State)){0};
    if (!source->live) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    AmaStatus status = AMA_NATIVE(prepare)(destination, source->count);
    if (status.code != AMA_OK) { return status; }
    memcpy(destination->limbs, source->limbs, source->count);
    destination->negative = source->negative;
    destination->live = true;
    return status;
}

/** Consume the live resource once; malformed caller lifetime never retries a free. */
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) {
    if (!state->live) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    AMA_INTEGER_RELEASE(state->limbs);
    *state = (AMA_NATIVE(State)){0};
    return ama_status_success();
}
